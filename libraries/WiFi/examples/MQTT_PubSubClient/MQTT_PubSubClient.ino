// MQTT con la libreria PubSubClient (instalela desde el Gestor de librerias)
// Publica la temperatura interna cada 10 s y escucha el tema "w80x/led".
#include <WiFi.h>
#include <PubSubClient.h>

const char *ssid     = "MiRed";
const char *password = "MiClave";
const char *broker   = "test.mosquitto.org";

WiFiClient red;
PubSubClient mqtt(red);

void mensaje(char *tema, byte *datos, unsigned int largo) {
  Serial.printf("Mensaje en %s: %.*s\n", tema, largo, (char *)datos);
  if (largo > 0) digitalWrite(LED_BUILTIN, datos[0] == '1' ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println(WiFi.localIP());
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mensaje);
}

void loop() {
  if (!mqtt.connected()) {
    String id = "w80x-" + WiFi.macAddress();
    if (mqtt.connect(id.c_str())) {
      mqtt.subscribe("w80x/led");
      Serial.println("Conectado al broker");
    } else {
      delay(2000);
      return;
    }
  }
  mqtt.loop();
  static uint32_t t;
  if (millis() - t > 10000) {
    t = millis();
    mqtt.publish("w80x/temperatura", String(temperatureRead(), 1).c_str());
  }
}
