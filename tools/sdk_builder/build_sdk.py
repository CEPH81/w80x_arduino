#!/usr/bin/env python3
"""
build_sdk.py - Compila el WM IoT SDK para cada chip y deja en sdk/ lo que
necesita el nucleo Arduino (librerias, cabeceras, configuracion, script de
enlace, cargador de arranque y tabla de particiones).

Uso (Linux, con el compilador csky en el PATH y las dependencias de Python
del SDK instaladas):

    WM_IOT_SDK_PATH=/ruta/wm_iot_sdk python3 tools/sdk_builder/build_sdk.py [soc...]

Parte de w80x_arduino (WM IoT SDK). LGPL 2.1.
"""
import json, os, re, shlex, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
PLAT = os.path.dirname(os.path.dirname(HERE))
OUT = os.path.join(PLAT, 'sdk')
SDK = os.environ.get('WM_IOT_SDK_PATH') or sys.exit('defina WM_IOT_SDK_PATH')
WORK = os.environ.get('SDK_BUILD_DIR', '/tmp/w80x_sdk_build')

# chips con radio (WiFi/BLE)
RADIO = {'w800', 'w801s', 'w802', 'w803'}
SOCS = sys.argv[1:] or ['w800', 'w801s', 'w802', 'w806']

# Se enlazan completas: arranque, interrupciones, HAL, drivers, RTOS,
# arbol de dispositivos (usan secciones y funciones debiles).
WHOLE = ['xt804', 'wm_system', 'wm_soc', 'wm_hal', 'driver', 'freertos', 'wm_dt',
         'wm_common', 'wm_heap', 'wm_log', 'partition_table', 'wm_ft_param', 'wm_debug']


def run(cmd, cwd):
    r = subprocess.run(cmd, cwd=cwd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if r.returncode:
        print(r.stdout[-4000:])
        sys.exit('fallo: %s' % cmd)
    return r.stdout


def build(soc):
    proj = os.path.join(WORK, soc)
    shutil.rmtree(proj, ignore_errors=True)
    shutil.copytree(os.path.join(HERE, 'project'), proj)
    cfg = open(os.path.join(HERE, 'prj_common.config')).read()
    if soc in RADIO:
        cfg += open(os.path.join(HERE, 'prj_wifi.config')).read()
    open(os.path.join(proj, 'prj.config'), 'w').write(cfg)
    wm = 'python3 %s/tools/wm/wm.py' % SDK
    run(wm + ' set-soc ' + soc, proj)
    out = run(wm + ' build', proj)
    print('  ' + '\n  '.join(l for l in out.splitlines() if 'FLASH:' in l or 'SRAM:' in l))
    return os.path.join(proj, 'build')


def harvest(soc, b):
    dst = os.path.join(OUT, soc)
    shutil.rmtree(dst, ignore_errors=True)
    for d in ('lib', 'config', 'ld', 'img', 'device_table'):
        os.makedirs(os.path.join(dst, d))
    ninja = open(os.path.join(b, 'build.ninja')).read()

    # --- librerias, en el orden de enlace del SDK
    m = re.search(r'^build arduino_base\.elf:.*?\n  FLAGS = .*?\n  LINK_LIBRARIES = (.*?)\n', ninja, re.S | re.M)
    libs = [x for x in shlex.split(m.group(1)) if x.endswith('.a')]
    whole, other = [], []
    strip = 'csky-elfabiv2-strip --strip-debug'
    for l in libs:
        name = os.path.basename(l)[3:-2]
        if name == 'main':
            continue
        src = os.path.normpath(os.path.join(b, l))
        shutil.copy(src, os.path.join(dst, 'lib'))
        lib = os.path.join(dst, 'lib', os.path.basename(src))
        if name == 'wm_dt':
            # el arbol de dispositivos lo aporta cada variante de placa
            members = run('csky-elfabiv2-ar t ' + lib, dst).split()
            for mem in members:
                if mem.startswith('wm_dt_hw.c'):
                    run('csky-elfabiv2-ar d %s %s' % (lib, mem), dst)
        run('%s %s' % (strip, lib), dst)
        (whole if name in WHOLE else other).append('-l' + name)
    open(os.path.join(dst, 'libs_whole.txt'), 'w').write(' '.join(whole) + '\n')
    open(os.path.join(dst, 'libs.txt'), 'w').write(' '.join(other) + '\n')

    # --- configuracion, arbol de dispositivos y enlace
    for f in ('wmsdk_config.h', 'wmsdk.config'):
        shutil.copy(os.path.join(b, 'config', f), os.path.join(dst, 'config'))
    for f in os.listdir(os.path.join(b, 'device_table')):
        shutil.copy(os.path.join(b, 'device_table', f), os.path.join(dst, 'device_table'))
    shutil.copy(os.path.join(b, 'project', 'chip_layout.ld'), os.path.join(dst, 'ld'))
    shutil.copy(os.path.join(b, 'bootloader', 'bootloader.img'), os.path.join(dst, 'img'))
    shutil.copy(os.path.join(b, 'partition_table', 'partition_table.img'), os.path.join(dst, 'img'))

    # --- parametros de la imagen del programa (de la orden del SDK)
    p = re.search(r'elf2bin\.py \S+ \S+ (0x[0-9A-Fa-f]+).*?bin2img\.py --input-binary \S+arduino_base\.bin .*?'
                  r'--image-header (\S+) --run-address (\S+).*?--version-string (\S+) --bootloader-config-flags (\S+)', ninja, re.S)
    img = dict(max=p.group(1), header=p.group(2), run=p.group(3), ver=p.group(4), flags=p.group(5))
    open(os.path.join(dst, 'image.txt'), 'w').write(
        ''.join('%s=%s\n' % kv for kv in img.items()))

    # --- cabeceras: rutas de inclusion de un archivo del proyecto
    subprocess.run('ninja -t compdb > compile_commands.json', cwd=b, shell=True, check=True)
    cc = json.load(open(os.path.join(b, 'compile_commands.json')))
    main = [c for c in cc if c['file'].endswith('main/src/main.c')][0]
    incs = []
    for a in shlex.split(main['command']):
        if a.startswith('-I'):
            d = os.path.normpath(a[2:])
            if d.startswith(os.path.normpath(SDK)):
                rel = os.path.relpath(d, SDK)
                incs.append(rel)
                copy_headers(d, os.path.join(OUT, 'include', rel))
    open(os.path.join(dst, 'includes.txt'), 'w').write(
        ''.join('-iwithprefixbefore %s\n' % r for r in incs))
    print('  %d librerias (%d completas), %d rutas de cabeceras, imagen %s' % (
        len(whole) + len(other), len(whole), len(incs), img))


def copy_headers(src, dst):
    """Copia los .h de una ruta de inclusion (con sus subcarpetas)."""
    for root, dirs, files in os.walk(src):
        for f in files:
            if f.endswith(('.h', '.hpp', '.inc')):
                rel = os.path.relpath(root, src)
                d = os.path.join(dst, rel)
                os.makedirs(d, exist_ok=True)
                shutil.copy(os.path.join(root, f), d)


if __name__ == '__main__':
    os.makedirs(WORK, exist_ok=True)
    for soc in SOCS:
        print('== %s' % soc)
        harvest(soc, build(soc))
    shutil.copy(os.path.join(SDK, 'LICENSE'), os.path.join(OUT, 'LICENSE-WM_IoT_SDK'))
    open(os.path.join(OUT, 'VERSION'), 'w').write(open(os.path.join(SDK, 'version')).read())
