# W80x IoT Core by MikrocontrollerCephs

**Núcleo Arduino (IDE 2.x) para los microcontroladores WinnerMicro W800, W801, W802 y W806**,
hecho y mantenido por [MikrocontrollerCephs](https://www.youtube.com/@mikrocontrollersceph)
(canal de YouTube de electrónica y microcontroladores).
No es un producto oficial de WinnerMicro.

Funciona con los microcontroladores **WinnerMicro W80x** (CPU XT804 a 240 MHz),
construido sobre el SDK oficial **WM IoT SDK v2.4** (FreeRTOS) de WinnerMicro.
Es un *fork* de [w80x_arduino de board707](https://github.com/board707/w80x_arduino):
se mantienen las mismas placas y la misma estructura (`boards.txt`, `platform.txt`,
`cores/w806`, `variants/*`, `libraries`), pero el núcleo está reescrito sobre los
drivers del SDK nuevo.

> **Estado: versión de prueba 0.9.6.** Núcleo (GPIO, `Serial`…`Serial5`, tiempos, ADC, PWM,
> interrupciones, FreeRTOS), librerías Wire, SPI, EEPROM, Preferences y HardwareTimer, y **WiFi**
> (estación, punto de acceso, búsqueda de redes, TCP, UDP, MQTT). Todo compila para las 6 placas;
> probado en HLK-W801-KIT (carga, Blink, WiFi, servidor web, ADC, temperatura) y HLK-W806-KIT (carga, LED).

| Placa | Chip | Flash para el programa | WiFi/BLE |
|---|---|---|---|
| W800 (HLK-W800-KIT) | W800 | 1,9 MB | sí |
| W801 (HLK-W801-KIT) | W801S | 1,9 MB | sí |
| W802 | W802 | 1,9 MB | sí |
| W806 (HLK-W806-KIT) | W806 | 1 MB | no |
| Air101 | W800 | 1,9 MB | sí |
| Air103 | W806 | 1 MB | no |

## Instalación (Gestor de placas)

1. **Archivo → Preferencias → URLs adicionales del gestor de placas**:
   ```
   https://raw.githubusercontent.com/CEPH81/w80x_arduino/wm-iot-sdk/package_w80x_wmsdk_index.json
   ```
2. **Herramientas → Placa → Gestor de placas**, busque **W80x IoT Core** e instale.
   Se descargan también el compilador C-SKY y `wm_tool` (los mismos del paquete de board707).

Puede convivir con el paquete `w80x_duino` de board707: son paquetes distintos.
Compilador disponible para **Windows y Linux** (x86-64).

## Menús de la placa

- **Frecuencia de CPU**: 240 (por defecto), 160, 120, 80 o 40 MHz.
- **Optimización**: velocidad (`-O2`, por defecto), tamaño (`-Os`), máxima velocidad (`-O3`) o depuración (`-Og`). El SDK va compilado con `-O2`.
- **Método de carga**:
  - *Puerto serie (USB)*: `wm_tool` reinicia el chip por RTS y graba el `.fls`
    (bootloader + programa + tabla de particiones). No toca la calibración de RF ni NVS.
    Al terminar, `wm_img` saca al chip del modo de carga con la orden de reinicio del SDK,
    así el programa arranca solo, sin pulsar RST. Al final de la carga se ve
    *«Chip reiniciado: programa en marcha.»*
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

## Librerías incluidas

| Librería | Qué hace | Ejemplos |
|---|---|---|
| **Wire** | I²C maestro hasta 400 kHz; `Wire.begin(sda, scl)` para otras patas; códigos de error de `endTransmission()` (2 = sin respuesta, 5 = bus bloqueado) | `i2c_scanner` |
| **SPI** | SPI maestro hasta 20 MHz, `SPISettings`, modos 0–3, MSB/LSB primero | `SPI_loopback` |
| **EEPROM** | EEPROM emulada en la flash (NVS), 2 KB por defecto (hasta 8 KB); se guarda al momento | `EEPROM_basico` |
| **Preferences** | ajustes con nombre, como en el ESP32 (`putInt`, `getString`…) | `Contador` |
| **HardwareTimer** | 6 temporizadores en µs o ms con interrupción (misma interfaz que antes) | `Tim_Led_Irq` |
| **WiFi** | `WiFi.begin`, `softAP`, `scanNetworks`, `WiFiClient`, `WiFiServer`, `WiFiUDP` (interfaz del ESP32). Solo W800, W801, W802 y Air101 | `WiFiScan`, `WiFiClientBasic`, `WiFiWebServer`, `WiFiAccessPoint`, `WiFiUDPNtp`, `MQTT_PubSubClient` |
| **BasicsExamples** | ejemplos del núcleo: Blink, Autoprueba, ADC, calibración, temperatura, PWM, servo, interrupciones y **puerto serie** | `Serial_Eco`, `Serial_Comandos` (órdenes de texto con `serialEvent()`), `Serial_Puente` (`Serial1` en PB6/PB7 hacia un módulo) |
| LiquidCrystal, SD, GyverOLED | las de board707, sin cambios (salvo detectar la placa) | las suyas |

Las patas por defecto de cada puerto están al principio de `variants/<placa>/pins_arduino.h`.

### WiFi en breve

```cpp
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.begin("MiRed", "MiClave");          // reconecta solo si se cae
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
}
```

- IP fija: `WiFi.config(IPAddress(192,168,1,50), IPAddress(192,168,1,1), IPAddress(255,255,255,0))` antes o después de `begin`.
- Punto de acceso: `WiFi.softAP("Nombre", "clave8+")`; la placa queda en **192.168.10.1** y reparte direcciones por DHCP.
  El canal lo elige el SDK (el de la red a la que esté conectada, o el 1).
- MQTT: instale **PubSubClient** desde el Gestor de librerías y vea el ejemplo `MQTT_PubSubClient`.
- Un programa con WiFi ocupa unos 560 KB (cabe de sobra en los 1,9 MB).

## ADC y temperatura interna

- **Entrada de 0 a 2,4 V como máximo.** Para tensiones mayores use un divisor de resistencias.
  Las lecturas por encima se dan como 2400 mV (o 4095).
- `analogReadMilliVolts(A0)`: milivoltios con la calibración de fábrica del chip (si la trae;
  `analogFactoryCalibrated()` lo dice) y la propia, si se hizo.
- `analogReadAveraging(16)`: promedia varias muestras en cada lectura (menos ruido).
- **Calibración propia de dos puntos**: ejemplo *BasicsExamples → ADC_Calibracion*, con un multímetro.
  Se guarda en la flash y se aplica en todos los programas. `analogCalibrationReset()` la borra.
- `temperatureRead()`: temperatura del chip en °C (ejemplo *Temperatura_interna*). Mide el silicio:
  suele dar unos grados más que el ambiente.
- `analogReadVddMilliVolts()`: tensión de alimentación del chip.

## Rendimiento

- Interrupciones de GPIO: despachador propio en RAM que solo atiende las patas con interrupción pendiente.
- `digitalWrite`/`digitalRead`: acceso directo a registros.
- WiFi: `WiFiClient` lee por bloques (cada llamada a lwIP es un mensaje entre tareas) y los
  servidores responden sin el retardo de Nagle (`setNoDelay(true)` por defecto).

## Diferencias con el núcleo anterior (board707)

- El programa corre en una tarea de FreeRTOS (`loopTask`, 8 KB de pila). Se pueden crear
  tareas con `xTaskCreate()`, colas, semáforos… `delay()` cede la CPU a otras tareas.
- `analogRead()` devuelve un valor de **12 bits** (0…4095, cambiable con
  `analogReadResolution()`); `analogReadMilliVolts()` devuelve milivoltios (antes
  `analogRead()` devolvía milivoltios). Entrada de 0 a 2,4 V.
- `analogWrite(pata, 0…255)`: PWM, 1 kHz por defecto. `analogWriteFrequency(hz)`: hasta 156 kHz
  con 255 pasos; por encima, menos pasos (máximo 20 MHz). `analogWriteGetFrequency()` da la real.
  Las patas que comparten canal llevan la misma señal (ver `variants/<placa>/pins_arduino.h`).
- `attachInterrupt(pata, función, RISING | FALLING | CHANGE | ONLOW | ONHIGH)`.
- `Serial.printf()`, `Serial1`…`Serial5` con las patas por defecto del SDK (ver
  `pins_arduino.h` de cada placa) o con otras: `Serial2.begin(9600, SERIAL_8N1, rx, tx)`.
- `temperatureRead()`, `getFreeHeap()`, `random()` por hardware.
- `EEPROM` ya no necesita crear un objeto: use `EEPROM.read(0)` (se puede seguir creando
  `EEPROMClass mi_eeprom(tamaño)` como antes).
- Se quitaron las librerías y ejemplos que dependían del HAL anterior (HAL_Examples, I2S,
  ILI9341_Fast_SDIO_SPI, prueba de LVGL y PSRAM).
- W802: la placa no tiene LED de usuario; `LED_BUILTIN` es la luz del conector TFT (PB8).

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
