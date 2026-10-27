# W80x (WM IoT SDK) para Arduino IDE 2.x

Núcleo Arduino para los microcontroladores **WinnerMicro W80x** (CPU XT804 a 240 MHz),
construido sobre el SDK oficial **WM IoT SDK v2.4** (FreeRTOS) de WinnerMicro.
Es un *fork* de [w80x_arduino de board707](https://github.com/board707/w80x_arduino):
se mantienen las mismas placas y la misma estructura (`boards.txt`, `platform.txt`,
`cores/w806`, `variants/*`, `libraries`), pero el núcleo está reescrito sobre los
drivers del SDK nuevo.

> **Estado: versión de prueba 0.9.0 (fase 1).** Funcionan GPIO, `Serial`…`Serial5`,
> `millis`/`micros`/`delay`, ADC, PWM, interrupciones y tareas de FreeRTOS.
> Las librerías (Wire, SPI, EEPROM, temporizadores) y el WiFi llegan en las fases 2 y 3.

| Placa | Chip | Flash para el programa | WiFi/BLE |
|---|---|---|---|
| W800 (HLK-W800-KIT) | W800 | 1,9 MB | sí (fase 3) |
| W801 (HLK-W801-KIT) | W801S | 1,9 MB | sí (fase 3) |
| W802 | W802 | 1,9 MB | sí (fase 3) |
| W806 (HLK-W806-KIT) | W806 | 1 MB | no |
| Air101 | W800 | 1,9 MB | sí (fase 3) |
| Air103 | W806 | 1 MB | no |

## Instalación (Gestor de placas)

1. **Archivo → Preferencias → URLs adicionales del gestor de placas**:
   ```
   https://raw.githubusercontent.com/CEPH81/w80x_arduino/wm-iot-sdk/package_w80x_wmsdk_index.json
   ```
2. **Herramientas → Placa → Gestor de placas**, busque **W80x (WM IoT SDK)** e instale.
   Se descargan también el compilador C-SKY y `wm_tool` (los mismos del paquete de board707).

Puede convivir con el paquete `w80x_duino` de board707: son paquetes distintos.
Compilador disponible para **Windows y Linux** (x86-64).

## Menús de la placa

- **Frecuencia de CPU**: 240 (por defecto), 160, 120, 80 o 40 MHz.
- **Optimización**: tamaño (`-Os`), velocidad (`-O2`/`-O3`) o depuración (`-Og`).
- **Método de carga**:
  - *Puerto serie (USB)*: `wm_tool` reinicia el chip por RTS y graba el `.fls`
    (bootloader + programa + tabla de particiones). No toca la calibración de RF ni NVS.
  - *Depurador CK-Link*: graba por JTAG con **XuanTie (T-Head) Debugger Server**
    (ver más abajo). No necesita puerto.
- **Velocidad de carga**: 2 M (por defecto) a 115200 baudios. Si falla, baje la velocidad.
- **Borrar flash**: solo el programa, o toda la flash (se conserva la calibración de RF).
- **Mensajes del SDK**: nivel de los mensajes del sistema por `Serial` (por defecto, solo avisos y errores).

> La primera carga en una placa que tenía firmware del núcleo anterior (o de fábrica)
> debe hacerse por **puerto serie**: el SDK nuevo usa otro bootloader y otra tabla de particiones.

### Carga con CK-Link

1. Instale **XuanTie Debugger Server** (T-Head DebugServer) de la web de XuanTie/OCC.
   Trae los algoritmos de flash de WinnerMicro (`…W800…FLASH…elf`, `…W806…FLASH…elf`).
2. Conexión: **TCK ↔ PA1**, **TMS ↔ PA4**, **GND ↔ GND**, y opcionalmente **TRST ↔ RST** y 3V3.
3. En **Método de carga** elija *Depurador CK-Link* y pulse **Subir**.

El programa `wm_img` busca `DebugServerConsole` en el PATH y en las carpetas de
instalación habituales; si lo tiene en otra, ponga esa carpeta en la variable de
entorno `XUANTIE_DEBUGSERVER`. Se graba la flash desde `0x08002000` (bootloader,
tabla de particiones y programa) y el chip se reinicia.

## Diferencias con el núcleo anterior (board707)

- El programa corre en una tarea de FreeRTOS (`loopTask`, 8 KB de pila). Se pueden crear
  tareas con `xTaskCreate()`, colas, semáforos… `delay()` cede la CPU a otras tareas.
- `analogRead()` devuelve un valor de **12 bits** (0…4095, cambiable con
  `analogReadResolution()`); `analogReadMilliVolts()` devuelve milivoltios (antes
  `analogRead()` devolvía milivoltios). Entrada de 0 a 2,4 V.
- `analogWrite(pata, 0…255)`: PWM de 8 bits, 1 kHz por defecto (`analogWriteFrequency()`).
  Las patas que comparten canal llevan la misma señal (ver `variants/<placa>/pins_arduino.h`).
- `attachInterrupt(pata, función, RISING | FALLING | CHANGE | ONLOW | ONHIGH)`.
- `Serial.printf()`, `Serial1`…`Serial5` con las patas por defecto del SDK (ver
  `pins_arduino.h` de cada placa), `temperatureRead()`, `getFreeHeap()`, `random()` por hardware.

## Para desarrolladores

- `tools/sdk_builder/build_sdk.py`: compila WM IoT SDK para cada chip y deja en `sdk/`
  las librerías, cabeceras, configuración, script de enlace, bootloader y tabla de particiones.
- `tools/sdk_builder/gen_variants.py`: genera el árbol de dispositivos (`wm_dt_hw.c`),
  `pins_mux.h` y `pins_arduino.h` de cada placa.
- `tools/sdk_builder/gen_boards.py`: genera `boards.txt`.
- `tools/wm_img`: herramienta en Go que crea `.img`, `.fls` y la imagen para CK-Link
  (sin Python) y lanza la carga por CK-Link.
- `tools/paquete.py`: crea el paquete y el índice del Gestor de placas.

## Licencias

Núcleo: LGPL 2.1 (como Arduino y board707). WM IoT SDK: Apache 2.0 (`sdk/LICENSE-WM_IoT_SDK`).
La documentación original de board707 está en `README_board707.md`.
