#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <ArduinoMDNS.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

#define WIFI_SSID "OrchestreFIMAV"
#define WIFI_PASS "OrchestreFIMAV"
#define WIFI_DELAY 1000
#define MQTT_BROKER_HOSTNAME "OrchestreFIMAV"
#define MQTT_PORT 1884
#define MQTT_USERNAME "instrument"
#define MQTT_PASSWORD "Instrument1234"
#define MQTT_TOPIC "fimav/orchestre"

const int br1_tl4 = 14; // 69
const int br2_tl5 = 15; // 71
const int br3_bl8 = 18; // 73
const int br4_bl6 = 20; // 78

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
WiFiUDP udp;
MDNS mdns(udp);

bool mqttConnected = false;

void connectMQTT(const char *name, IPAddress ip);

void setup()
{
  Serial.begin(9600);

  pinMode(br1_tl4, OUTPUT);
  pinMode(br2_tl5, OUTPUT);
  pinMode(br3_bl8, OUTPUT);
  pinMode(br4_bl6, OUTPUT);
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connexion au réseau WiFi en cours...");
    delay(WIFI_DELAY);
  }

  Serial.println("Vous êtes connecté!");
  Serial.println("Adresse IP: ");
  Serial.println(WiFi.localIP());

  mdns.begin(WiFi.localIP());
  mdns.setNameResolvedCallback(connectMQTT);
  mdns.resolveName(MQTT_BROKER_HOSTNAME, 5000);

  IPAddress brokerIP;
  if (!WiFi.hostByName(MQTT_BROKER_HOSTNAME, brokerIP)) {
    Serial.println("Erreur : impossible de résoudre le nom du broker.");
    return;
  }
  connectMQTT(MQTT_BROKER_HOSTNAME, brokerIP);
}

void loop()
{
  if (!mqttConnected) {
    mdns.run();
  }
  else if (mqttClient.parseMessage()) {
    String topic = mqttClient.messageTopic();
    String message;

    while (mqttClient.available()) {
      char c = (char)mqttClient.read();
      message += c;
    }

    Serial.print("Note reçue sur le topic [");
    Serial.print(topic);
    Serial.print("] : ");
    Serial.println(message);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
      Serial.print("Erreur de parsing JSON : ");
      Serial.println(error.f_str());
      return;
    }

    const char* event = doc["event"];
    int channel = doc["channel"];
    int note = doc["note"];

    Serial.println(event);
    Serial.println(channel);
    Serial.println(note);

    if (channel != 4) {
      return;
    }

    if (note == 67) {
      Serial.print("→ Note 67 reçue ! Status: ");

      if (strcmp(event, "note_on") == 0) {
        Serial.println("on");
        digitalWrite(br1_tl4, HIGH);
      }
      else if (strcmp(event, "note_off") == 0) {
        Serial.println("off");
        digitalWrite(br1_tl4, LOW);
      }
    } else if (note == 71) {
      Serial.println("→ Note 71 reçue !");

      if (strcmp(event, "note_on") == 0) {
        Serial.println("on");
        digitalWrite(br2_tl5, HIGH);
      }
      else if (strcmp(event, "note_off") == 0) {
        Serial.println("off");
        digitalWrite(br2_tl5, LOW);
      }
    } else if (note == 73) {
      Serial.println("→ Note 73 reçue !");

      if (strcmp(event, "note_on") == 0) {
        Serial.println("on");
        digitalWrite(br3_bl8, HIGH);
      }
      else if (strcmp(event, "note_off") == 0) {
        Serial.println("off");
        digitalWrite(br3_bl8, LOW);
      }
    } else if (note == 78) {
      Serial.println("→ Note 78 reçue !");

      if (strcmp(event, "note_on") == 0) {
        Serial.println("on");
        digitalWrite(br4_bl6, HIGH);
      }
      else if (strcmp(event, "note_off") == 0) {
        Serial.println("off");
        digitalWrite(br4_bl6, LOW);
      }
    }
  }
}

/**
 * @brief Callback pour la résolution du broker via mDNS
 * @param name Nom du broker MQTT
 * @param ip Adresse IP du broker MQTT obtenue
 */
void connectMQTT(const char *name, IPAddress ip)
{
  if (ip == INADDR_NONE)
  {
    Serial.println("Il y a eu une erreur lors de l'obtention de l'adresse IP du broker MQTT!");
    return;
  }

  mqttClient.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);

  Serial.println("Connexion au broker MQTT avec l'adresse IP :");
  
  Serial.print("IP : ");
  Serial.println(ip);
  Serial.print("Port : ");
  Serial.println(MQTT_PORT);
  
  if (!mqttClient.connect(ip, MQTT_PORT))
  {
    Serial.print("Échec de la connexion au broker MQTT! Code d'erreur : ");
    Serial.println(mqttClient.connectError());
  }
  else
  {
    Serial.println("Connexion au broker MQTT réussie!");
    mqttClient.subscribe(MQTT_TOPIC);
    mqttConnected = true;
  }
}