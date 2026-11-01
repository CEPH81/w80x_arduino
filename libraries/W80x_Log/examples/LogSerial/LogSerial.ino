/*
 * LogSerial - puerto serie con log en color: ordenes por teclado y respuestas por niveles.
 *
 * Abra una terminal con colores ANSI (PuTTY, Tera Term, VS Code) a 115200 baudios
 * y escriba una orden + Enter:
 *   on / off      LED encendido / apagado           (verde)
 *   temp          temperatura del chip              (verde, amarilla si pasa de 50 C)
 *   adc           A0 en mV (0 a 2,4 V)              (verde, roja si satura)
 *   hex <texto>   volcado hexadecimal del texto     (azul)
 *   nivel e|w|i|d|v   cambia el nivel del log
 *   color on|off  colores ANSI (off para el Monitor serie del Arduino IDE)
 *   ayuda
 * Una orden desconocida sale en amarillo; un numero fuera de rango, en rojo.
 * Sirve para todas las placas W80x.
 */
#include <W80x_Log.h>

const int led = LED_BUILTIN;
String linea;

void ayuda() {
  LOGI("AYUDA", "on | off | temp | adc | hex <texto> | nivel e|w|i|d|v | color on|off");
}

void ejecutar(String o) {
  o.trim();
  String cmd = o, arg = "";
  int sp = o.indexOf(' ');
  if (sp > 0) { cmd = o.substring(0, sp); arg = o.substring(sp + 1); }
  cmd.toLowerCase();
  LOGD("RX", "orden \"%s\" argumento \"%s\"", cmd.c_str(), arg.c_str());

  if (cmd == "on")       { digitalWrite(led, HIGH); LOGI("LED", "encendido"); }
  else if (cmd == "off") { digitalWrite(led, LOW);  LOGI("LED", "apagado"); }
  else if (cmd == "temp") {
    float t = temperatureRead();
    if (t > 50) LOGW("TEMP", "%.1f C (caliente)", t);
    else        LOGI("TEMP", "%.1f C", t);
  } else if (cmd == "adc") {
    uint32_t mv = analogReadMilliVolts(A0);
    if (mv >= 2400) LOGE("ADC", "A0 saturado (>= 2400 mV): use un divisor");
    else            LOGI("ADC", "A0 = %u mV", mv);
  } else if (cmd == "hex") {
    Log_Hex(LOG_DEBUG, "HEX", arg.c_str(), arg.length());
  } else if (cmd == "nivel") {
    const char *n = "ewidv";
    const char *p = arg.length() ? strchr(n, tolower(arg[0])) : NULL;
    if (!p) { LOGE("LOG", "nivel no valido: use e, w, i, d o v"); return; }
    Log_SetLevel((LOG_LevelTypeDef)(p - n + 1));
    Log_Write(LOG_ERROR, "LOG", "nivel cambiado a %c", *p);
  } else if (cmd == "color") {
    Log_SetColors(arg != "off");
    LOGI("LOG", "colores %s", arg == "off" ? "desactivados" : "activados");
  } else if (cmd == "ayuda" || cmd == "?") {
    ayuda();
  } else if (cmd.length()) {
    LOGW("RX", "orden desconocida: %s", cmd.c_str());
  }
}

void setup() {
  Log_Init(115200);
  Log_SetLevel(LOG_DEBUG);
  pinMode(led, OUTPUT);
  delay(300);
  LOGI("MAIN", "LogSerial listo (CPU %u MHz)", getCpuFrequencyMhz());
  ayuda();
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (linea.length()) ejecutar(linea);
      linea = "";
    } else if (linea.length() < 64) {
      linea += c;
      Serial.write(c);   // eco para terminales sin eco local
    }
  }
}
