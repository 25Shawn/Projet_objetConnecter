#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>

// ====================== CONFIG ======================
// WiFi
char ssid[] = "Domotique-Pedago";
char pass[] = "domoinfo36";

const int broche_data = 13;
// MQTT
const char* mqtt_server = "172.19.240.7";  // IP du broker
const int mqtt_port = 1883;
const char* mqtt_topic = "musique/notes";

// Clients WiFi & MQTT
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ====================================================

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Note reçue: ");
  Serial.println(message);

  if (message == "DO") {
    Serial.println("→ Note DO reçue !");
    digitalWrite(broche_data, HIGH);
    delay(5000);
    digitalWrite(broche_data, LOW);
  } else if (message == "RE") {
    Serial.println("→ Note RE reçue !");
  } else if (message == "MI") {
    Serial.println("→ Note MI reçue !");
  } else {
    Serial.println("→ Note inconnue.");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connexion MQTT...");
    if (client.connect("ArduinoClient")) {
      Serial.println("connecté !");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 sec...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Connexion WiFi
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connecté");

  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
