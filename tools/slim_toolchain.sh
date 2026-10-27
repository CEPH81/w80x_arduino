#!/bin/bash
# slim_toolchain.sh <carpeta_toolchain_extraida> <salida.tar.bz2> <nombre_raiz>
#
# Reempaqueta el compilador C-SKY (csky-elfabiv2, minilibc 20210423) con una
# sola carpeta raiz (lo exige el Gestor de placas del IDE 2.x) y solo con lo
# que usa el W80x: multilib ck803/hard-fp (la que elige -mcpu=ck804ef
# -mhard-float), sin big-endian, sin otras CPU y sin documentacion.
#
# Parte de w80x_arduino. LGPL 2.1.
set -e
SRC=$1; OUT=$2; ROOT=$3
TMP=$(mktemp -d)
cp -a "$SRC" "$TMP/$ROOT"
T="$TMP/$ROOT"
for d in csky-elfabiv2/lib lib/gcc/csky-elfabiv2/6.3.0 csky-elfabiv2/include/c++/6.3.0/csky-elfabiv2; do
    [ -d "$T/$d" ] || continue
    for m in big ck801 ck802 ck805 ck807 ck810v ck860 ck860v soft-fp hard-fp; do
        rm -rf "$T/$d/$m"
    done
    [ -d "$T/$d/ck803" ] && rm -rf "$T/$d/ck803/soft-fp"
done
rm -rf "$T/share/info" "$T/share/man" "$T/share/locale" "$T/share/doc"
( cd "$TMP" && tar --sort=name --owner=0 --group=0 --numeric-owner --mtime='2026-01-01' -cjf - "$ROOT" ) > "$OUT"
rm -rf "$TMP"
ls -la "$OUT"
