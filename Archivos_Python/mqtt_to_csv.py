#      MÓDULOS DE COMUNICACIÓN, PARSEO Y ALMACENAMIENTO DE DATOS
import paho.mqtt.client as mqtt
import csv
import json
from datetime import datetime

# CONFIGURACIÓN MQTT
BROKER = "c02751a84b9f4525b8321654975122c8.s1.eu.hivemq.cloud"
PORT = 8883
USERNAME = "esp32_user"
PASSWORD = "Esp32_pass"
TOPIC = "bus/data"

# ARCHIVO CSV
CSV_FILE = "dataset.csv"

# Creación de archivo con cabecera si no existe
try:
    with open(CSV_FILE, 'x', newline='') as f:
        writer = csv.writer(f)
        writer.writerow([
            "timestamp", "lat", "lon",
            "temp", "hum", "air",
            "aforo", "redes", "rssi"
        ])
except FileExistsError:
    pass


# CALLBACK AL RECIBIR DATOS
def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode()
        data = json.loads(payload)

        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        row = [
            timestamp,
            data.get("lat"),
            data.get("lon"),
            data.get("temp"),
            data.get("hum"),
            data.get("air"),
            data.get("aforo"),
            data.get("redes"),
            data.get("rssi")
        ]

        print("Datos recibidos:", row)

        with open(CSV_FILE, 'a', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(row)

    except Exception as e:
        print("Error:", e)

# CONEXIÓN MQTT
client = mqtt.Client()

client.username_pw_set(USERNAME, PASSWORD)

# TLS requerido por HiveMQ Cloud
client.tls_set()

client.on_message = on_message

print("Conectando a MQTT...")
client.connect(BROKER, PORT)

client.subscribe(TOPIC)

print("Escuchando datos...")

client.loop_forever()