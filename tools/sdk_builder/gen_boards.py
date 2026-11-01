#!/usr/bin/env python3
"""gen_boards.py - genera boards.txt (todas las placas con los mismos menus).
Parte de w80x_arduino. LGPL 2.1."""
import os

PLAT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
# id, nombre, variante, chip SDK, flash maxima del programa
BOARDS = [
    ('w800',   'W800 (HLK-W800-KIT)',  'w800',   'w800',  0x1E0C00),
    ('w801',   'W801 (HLK-W801-KIT)',  'w801',   'w801s', 0x1E0C00),
    ('w802',   'W802',                 'w802',   'w802',  0x1E0C00),
    ('w806',   'W806 (HLK-W806-KIT)',  'w806',   'w806',  0xFFC00),
    ('air101', 'Air101',               'air101', 'w800',  0x1E0C00),
    ('air103', 'Air103',               'air103', 'w806',  0xFFC00),
]
CPU = [240, 160, 120, 80, 40]
OPT = [('o2', 'Velocidad (-O2)', '-O2'), ('size', 'Tamano (-Os)', '-Os'),
       ('o3', 'Maxima velocidad (-O3)', '-O3'), ('debug', 'Depuracion (-Og)', '-Og')]
SPEED = [2000000, 1000000, 921600, 460800, 115200]
LOG = [('warn', 'Avisos y errores', 2), ('none', 'Ninguno', 0), ('error', 'Errores', 1),
       ('info', 'Informacion', 3), ('debug', 'Depuracion', 4)]

out = ['# Placas W80x (WM IoT SDK). Generado por tools/sdk_builder/gen_boards.py', '',
       'menu.cpu_frq=Frecuencia de CPU', 'menu.opt=Optimizacion', 'menu.upload_method=Metodo de carga',
       'menu.upload_speed=Velocidad de carga', 'menu.erase=Borrar flash', 'menu.log=Mensajes del SDK', '']
for bid, name, var, soc, mx in BOARDS:
    b = lambda k, v: out.append('%s.%s=%s' % (bid, k, v))
    out.append('#' * 70)
    b('name', name)
    b('build.board', bid.upper())
    b('build.core', 'w806')
    b('build.variant', var)
    b('build.soc', soc)
    b('build.mcu', 'ck804ef')
    b('upload.maximum_size', mx)
    b('upload.maximum_data_size', 163840)
    b('upload.erase', '')
    b('serial.disableRTS', 'true')
    b('serial.disableDTR', 'true')
    for f in CPU:
        b('menu.cpu_frq.%dMHZ' % f, '%d MHz' % f)
        b('menu.cpu_frq.%dMHZ.build.f_cpu' % f, '%d000000L' % f)
    for k, n, f in OPT:
        b('menu.opt.%s' % k, n)
        b('menu.opt.%s.build.flags.optimize' % k, f)
    b('menu.upload_method.serial', 'Puerto serie (USB)')
    b('menu.upload_method.serial.upload.tool', 'serial_upload')
    b('menu.upload_method.serial.upload.protocol', 'serial')
    b('menu.upload_method.cklink', 'Depurador CK-Link (XuanTie Debugger Server)')
    b('menu.upload_method.cklink.upload.tool', 'cklink')
    b('menu.upload_method.cklink.upload.protocol', 'cklink')
    b('menu.upload_method.cklink.upload.require_upload_port', 'false')
    b('menu.upload_method.cklink.upload.use_1200bps_touch', 'false')
    b('menu.upload_method.cklink.upload.wait_for_upload_port', 'false')
    for s in SPEED:
        b('menu.upload_speed.%d' % s, '%d' % s)
        b('menu.upload_speed.%d.upload.speed' % s, s)
    b('menu.erase.no', 'Solo el programa')
    b('menu.erase.all', 'Toda la flash (conserva la calibracion de RF)')
    b('menu.erase.all.upload.erase', '-eo all')
    for k, n, v in LOG:
        b('menu.log.%s' % k, n)
        b('menu.log.%s.build.log_level' % k, v)
    out.append('')
open(os.path.join(PLAT, 'boards.txt'), 'w').write('\n'.join(out) + '\n')
print('boards.txt: %d placas' % len(BOARDS))
