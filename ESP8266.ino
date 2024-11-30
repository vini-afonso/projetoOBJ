#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Configurações do Wi-Fi
const char* ssid = "Amalia_2.4G";         // Substitua pelo nome da sua rede Wi-Fi
const char* password = "Nylon123!";   // Substitua pela senha do Wi-Fi

// Configurações do Broker MQTT (HiveMQ Cloud)
const char* mqtt_server = "f99d2908acd74c748a84b79f1a625aec.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "OBJServer";       // Substitua pelo nome de usuário do cluster HiveMQ
const char* mqtt_password = "Obj741852";    // Substitua pela senha do cluster HiveMQ

// Tópicos MQTT
const char* temperature_topic = "sensor/temperature";
const char* fan_control_topic = "actuator/fan";

// Cliente TLS e MQTT
WiFiClientSecure espClient; // Cliente com suporte a TLS
PubSubClient client(espClient);

// Variáveis globais
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 5000; // Intervalo de publicação (5 segundos)

void setup_wifi() {
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  if (strcmp(topic, fan_control_topic) == 0) {
    Serial.print("Comando para o ventilador recebido: ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();

    // Repassa o comando ao Arduino
    Serial.print("fan:");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao broker MQTT com TLS...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado ao broker MQTT com TLS!");
      client.subscribe(fan_control_topic);
    } else {
      Serial.print("Falha, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configuração Wi-Fi
  setup_wifi();

  // Configuração do cliente MQTT
  espClient.setInsecure(); // Não valida o certificado do servidor (usar apenas para testes)
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Recebe temperatura do Arduino via Serial
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    if (data.startsWith("temperature:")) {
      String temperature = data.substring(12);
      if (client.publish(temperature_topic, temperature.c_str())) {
        Serial.print("Temperatura publicada: ");
        Serial.println(temperature);
      } else {
        Serial.println("Falha ao publicar temperatura.");
      }
    }
  }

  yield(); // Previne Watchdog Timer Reset
}