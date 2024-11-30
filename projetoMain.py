import serial
import time
import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import numpy as np

# Configuração do Broker MQTT
broker = "f99d2908acd74c748a84b79f1a625aec.s1.eu.hivemq.cloud"
port = 8883
username = "OBJServer"
password = "Obj741852"

# Configuração da Porta Serial
serial_port = "COM4"  # Altere para a porta correta do Arduino
baud_rate = 9600

# Inicializa a Conexão Serial
arduino = serial.Serial(serial_port, baud_rate, timeout=2)
time.sleep(2)  # Aguarda a inicialização da comunicação serial

# Configuração do Cliente MQTT
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado ao Broker MQTT!")
    else:
        print(f"Falha na conexão, código: {rc}")

def on_message(client, userdata, msg):
    print(f"Mensagem recebida: {msg.topic} -> {msg.payload.decode()}")

client = mqtt.Client()
client.username_pw_set(username, password)
client.tls_set()  # Ativa TLS para a conexão segura

client.on_connect = on_connect
client.on_message = on_message

# Conecta ao Broker MQTT
print("Conectando ao Broker MQTT...")
client.connect(broker, port)
client.loop_start()

# Dados para Gradiente e Fluxo de Calor
condutividade_termica = 200  # W/m·K (simulado)
posicoes = np.linspace(0, 10, 10)  # Posições simuladas ao longo de um comprimento
temperaturas = []  # Lista para armazenar as temperaturas
fluxos_de_calor = []
velocidades_ventoinha = []

# Função para calcular a velocidade da ventoinha (simulada)
def calcular_velocidade_ventoinha(temperatura):
    return int(min(max((temperatura - 20) * 5, 0), 255))  # Mapeia 20°C a 0% e temperaturas maiores a até 255

try:
    while True:
        # Lê os dados enviados pelo Arduino
        if arduino.in_waiting > 0:
            linha = arduino.readline().decode("utf-8").strip()
            if linha.startswith("temperature:"):
                # Extrai a temperatura
                temperatura = float(linha.split(":")[1])
                temperaturas.append(temperatura)

                # Calcular Gradiente de Temperatura
                if len(temperaturas) > 1:
                    gradiente_temperatura = (temperaturas[-1] - temperaturas[-2]) / (posicoes[-1] - posicoes[-2])
                else:
                    gradiente_temperatura = 0

                # Calcular Fluxo de Calor
                fluxo_de_calor = -condutividade_termica * gradiente_temperatura
                fluxos_de_calor.append(fluxo_de_calor)

                # Calcular Velocidade da Ventoinha
                velocidade_ventoinha = calcular_velocidade_ventoinha(temperatura)
                velocidades_ventoinha.append(velocidade_ventoinha)

                # Publica os dados no servidor MQTT
                mensagem = {
                    "temperatura": temperatura,
                    "gradiente_temperatura": gradiente_temperatura,
                    "fluxo_de_calor": fluxo_de_calor,
                    "velocidade_ventoinha": velocidade_ventoinha,
                }
                client.publish("sensor/temperature", str(mensagem))
                print(f"Mensagem publicada: {mensagem}")

        time.sleep(1)  # Atraso para leitura

except KeyboardInterrupt:
    print("\nFinalizando...")

finally:
    client.loop_stop()
    client.disconnect()
    arduino.close()

    # Plotagem dos Gráficos
    plt.figure(figsize=(12, 8))

    # Gráfico de Temperatura
    plt.subplot(3, 1, 1)
    plt.plot(temperaturas, label="Temperatura (°C)", color="blue")
    plt.title("Temperatura")
    plt.xlabel("Amostra")
    plt.ylabel("°C")
    plt.grid()
    plt.legend()

    # Gráfico de Fluxo de Calor
    plt.subplot(3, 1, 2)
    plt.plot(fluxos_de_calor, label="Fluxo de Calor (W/m²)", color="red")
    plt.title("Fluxo de Calor")
    plt.xlabel("Amostra")
    plt.ylabel("W/m²")
    plt.grid()
    plt.legend()

    # Gráfico de Velocidade da Ventoinha
    plt.subplot(3, 1, 3)
    plt.plot(velocidades_ventoinha, label="Velocidade da Ventoinha", color="green")
    plt.title("Velocidade da Ventoinha")
    plt.xlabel("Amostra")
    plt.ylabel("PWM")
    plt.grid()
    plt.legend()

    plt.tight_layout()
    plt.show()

    print("Conexão encerrada.")