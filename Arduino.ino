#include <DHT.h>

// Configuração do DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Configuração do ventilador
#define FAN_PIN 9

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(9600);  // Comunicação com o Python
  dht.begin();         // Inicializa o sensor DHT11

  // Configura o pino do ventilador como saída
  pinMode(FAN_PIN, OUTPUT);

  Serial.println("Arduino iniciado e pronto para enviar dados.");
}

void loop() {
  // Lê a temperatura do sensor
  float temperature = dht.readTemperature();
  if (!isnan(temperature)) {
    // Envia a temperatura via Serial
    Serial.print("temperature:");
    Serial.println(temperature);
  } else {
    Serial.println("Erro ao ler do sensor DHT11.");
  }

  // Recebe comandos para o ventilador via Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    if (command.startsWith("fan:")) {
      int fanSpeed = command.substring(4).toInt(); // Extrai o valor da velocidade
      analogWrite(FAN_PIN, fanSpeed);             // Ajusta a velocidade do ventilador
      Serial.print("Velocidade do ventilador ajustada para: ");
      Serial.println(fanSpeed);
    }
  }

  delay(2000); // Atraso para evitar sobrecarga na comunicação
}