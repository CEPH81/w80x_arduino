#!/usr/bin/env python3
"""
paquete.py - crea el paquete del Gestor de placas (Arduino IDE 2.x).

Genera en dist/:
  - w80x_wmsdk-<version>.tar.bz2      la plataforma
  - package_w80x_wmsdk_index.json     indice (plataforma + compilador + wm_tool)

El archivo es reproducible (orden y fecha fijos), asi que la suma SHA-256
solo cambia si cambia el contenido.

Publicacion:
  1. python3 tools/paquete.py
  2. subir el .tar.bz2 a la rama "paquetes" del repositorio
  3. copiar el .json a la raiz de la rama wm-iot-sdk y hacer push
  4. en el IDE, URL adicional del gestor de placas:
     https://raw.githubusercontent.com/CEPH81/w80x_arduino/wm-iot-sdk/package_w80x_wmsdk_index.json

Parte de w80x_arduino. LGPL 2.1.
"""
import bz2, hashlib, io, json, os, re, sys, tarfile

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DIST = os.path.join(RAIZ, 'dist')
REPO = 'CEPH81/w80x_arduino'
PAQUETE = 'w80x_wmsdk'
FECHA = 1767225600  # 2026-01-01

# lo que va en el paquete (rutas relativas a la raiz)
INCLUIR = ['boards.txt', 'platform.txt', 'LICENSE', 'README.md', 'cores', 'variants', 'sdk',
           'libraries', 'tools/wm_img/bin']
EXCLUIR = re.compile(r'(^|/)(platform\.local\.txt|\.git[^/]*|__pycache__|[^/]*\.(o|d|pyc))(/|$)')

# compilador C-SKY V3.10.29 minilibc (20210423), el mismo de board707, reempaquetado
# con una sola carpeta raiz y solo la multilib ck803/hard-fp (tools/slim_toolchain.sh)
CSKY_VER = "2021.04.23-slim"
CSKY_URL = "https://raw.githubusercontent.com/%s/paquetes/" % REPO
TOOLS = [
    {"name": "csky", "version": CSKY_VER, "systems": [
        {"host": "i686-mingw32",
         "url": CSKY_URL + "csky-elfabiv2-minilibc-20210423-windows.tar.bz2",
         "archiveFileName": "csky-elfabiv2-minilibc-20210423-windows.tar.bz2",
         "checksum": "SHA-256:97467def95c748f85d10d54cb08a1fe296a9fa003d9e9eedc019e121d255ec07",
         "size": "33400023"},
        {"host": "x86_64-pc-linux-gnu",
         "url": CSKY_URL + "csky-elfabiv2-minilibc-20210423-linux-x86_64.tar.bz2",
         "archiveFileName": "csky-elfabiv2-minilibc-20210423-linux-x86_64.tar.bz2",
         "checksum": "SHA-256:87516b16a04d725f3baeb9a0d6e5eb6f5501896c66035f76f52ff3644fe931b6",
         "size": "35128367"}]},
]


def leer(clave):
    for l in open(os.path.join(RAIZ, 'platform.txt'), encoding='utf-8'):
        if l.startswith(clave + '='):
            return l.split('=', 1)[1].strip()
    sys.exit('platform.txt sin ' + clave)


def placas():
    return [{"name": m.group(1)} for m in
            re.finditer(r'^\w+\.name=(.+)$', open(os.path.join(RAIZ, 'boards.txt'), encoding='utf-8').read(), re.M)]


def herramienta_wm_tool():
    """wm_tool: se toma la entrada del indice original (board707)."""
    orig = json.load(open(os.path.join(RAIZ, 'package_w80x_index.json'), encoding='utf-8'))
    for t in orig['packages'][0]['tools']:
        if t['name'] == 'w80x_tool' and t['version'] == 'v1.0':
            return t
    sys.exit('no se encuentra w80x_tool v1.0 en package_w80x_index.json')


def crear_tar(version):
    nombre = '%s-%s.tar.bz2' % (PAQUETE, version)
    tope = '%s-%s' % (PAQUETE, version)
    buf = io.BytesIO()
    with tarfile.open(fileobj=buf, mode='w', format=tarfile.GNU_FORMAT) as tar:
        archivos = []
        for inc in INCLUIR:
            ruta = os.path.join(RAIZ, inc)
            if os.path.isfile(ruta):
                archivos.append(inc)
            else:
                for r, ds, fs in os.walk(ruta):
                    ds.sort()
                    for f in sorted(fs):
                        archivos.append(os.path.relpath(os.path.join(r, f), RAIZ).replace(os.sep, '/'))
        for rel in sorted(archivos):
            if EXCLUIR.search(rel):
                continue
            ruta = os.path.join(RAIZ, rel)
            info = tar.gettarinfo(ruta, arcname=tope + '/' + rel)
            info.uid = info.gid = 0
            info.uname = info.gname = ''
            info.mtime = FECHA
            info.mode = 0o755 if os.access(ruta, os.X_OK) and '/bin/' in rel else 0o644
            with open(ruta, 'rb') as f:
                tar.addfile(info, f)
    datos = bz2.compress(buf.getvalue(), 9)
    os.makedirs(DIST, exist_ok=True)
    open(os.path.join(DIST, nombre), 'wb').write(datos)
    return nombre, len(datos), hashlib.sha256(datos).hexdigest()


def main():
    version = leer('version')
    nombre, tam, sha = crear_tar(version)
    deps = [{"packager": PAQUETE, "name": "csky", "version": CSKY_VER},
            {"packager": PAQUETE, "name": "w80x_tool", "version": "v1.0"}]
    indice = {"packages": [{
        "name": PAQUETE,
        "maintainer": "MikrocontrollerCephs (CEPH81) - basado en w80x_arduino de board707 y WM IoT SDK de WinnerMicro",
        "websiteURL": "https://github.com/%s/tree/wm-iot-sdk" % REPO,
        "email": "",
        "help": {"online": "https://github.com/%s/tree/wm-iot-sdk" % REPO},
        "platforms": [{
            "name": leer('name'),
            "architecture": "w80x",
            "version": version,
            "category": "Contributed",
            "url": "https://raw.githubusercontent.com/%s/paquetes/%s" % (REPO, nombre),
            "archiveFileName": nombre,
            "checksum": "SHA-256:" + sha,
            "size": str(tam),
            "help": {"online": "https://github.com/%s/tree/wm-iot-sdk" % REPO},
            "boards": placas(),
            "toolsDependencies": deps,
        }],
        "tools": TOOLS + [herramienta_wm_tool()],
    }]}
    ruta = os.path.join(DIST, 'package_%s_index.json' % PAQUETE)
    with open(ruta, 'w', encoding='utf-8') as f:
        json.dump(indice, f, indent=2, ensure_ascii=False)
        f.write('\n')
    print('Plataforma %s: %s (%d bytes, SHA-256 %s)' % (version, nombre, tam, sha))
    print('Indice: %s' % ruta)


if __name__ == '__main__':
    main()
