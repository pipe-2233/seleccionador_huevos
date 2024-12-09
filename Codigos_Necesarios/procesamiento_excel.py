import paho.mqtt.client as mqtt
import openpyxl
from datetime import datetime
from flask import Flask, send_file
import threading

# Dirección y nombre del archivo Excel
excel_file = r'C:\Users\andre\Documents\base de datos\conteo_huevos.xlsx'

# Configurar Flask
app = Flask(__name__)

# Ruta para descargar el archivo Excel
@app.route('/descargar_excel')
def descargar_excel():
    try:
        return send_file(excel_file, as_attachment=True)
    except Exception as e:
        return str(e)

# Conectar con el archivo Excel
def open_excel_file():
    try:
        wb = openpyxl.load_workbook(excel_file)
        sheet = wb.active
        return wb, sheet
    except Exception as e:
        print(f"Error al abrir el archivo Excel: {e}")
        return None, None

# Escribir datos en el archivo Excel
def write_to_excel(cell, cantidad):
    wb, sheet = open_excel_file()
    if wb is None:
        return

    # Obtener el valor actual en la celda
    current_value = sheet[cell].value or 0

    # Sumar la cantidad al valor actual
    new_value = current_value + cantidad

    # Escribir el nuevo conteo en la celda correspondiente
    sheet[cell] = new_value

    # Guardar el archivo Excel
    wb.save(excel_file)
    print(f"Datos escritos en la celda {cell}: {new_value} huevos.")

# Callback cuando el cliente se conecta al broker
def on_connect(client, userdata, flags, rc):
    print("Conectado con el código de resultado: " + str(rc))
    # Suscribirse a los temas "tipo_huevo", "clase_huevo", y "solicitar_excel"
    client.subscribe("tipo_huevo")
    client.subscribe("clase_huevo")
    client.subscribe("solicitar_excel")  # Suscripción al topic para descargar el archivo

# Callback cuando se recibe un mensaje
def on_message(client, userdata, msg):
    try:
        if msg.topic == "solicitar_excel":  # Cuando se recibe el mensaje del topic "solicitar_excel"
            print("Solicitud para descargar el archivo Excel recibida.")
            # Aquí simplemente activamos el servidor Flask para descargar el archivo
            # El cliente en Node-RED puede acceder al archivo a través del servidor Flask.
            pass  # El servidor Flask manejará la respuesta para la descarga.

        # Lógica para el topic "tipo_huevo" y "clase_huevo" sigue igual...
        if msg.topic == "tipo_huevo":
            payload = msg.payload.decode('utf-8').split(',')
            if len(payload) == 1:  # Si solo hay un tipo de huevo (como "A")
                tipo = payload[0]  # Tipo de huevo
                cantidad = 1  # Se suma 1 al conteo
            elif len(payload) == 2:  # Si hay tipo de huevo y cantidad
                tipo = payload[0]  # Tipo de huevo
                cantidad = int(payload[1])  # Cantidad de huevos
            else:
                raise ValueError("Formato de mensaje incorrecto")
            write_to_excel(tipo_to_cell(tipo), cantidad)

        elif msg.topic == "clase_huevo":
            payload = msg.payload.decode('utf-8')
            if payload in ["brow-egg-dirty", "white-egg-dirty"]:
                write_to_excel("H13", 1)
            elif payload in ["white-egg-crack", "brown-egg-crack"]:
                write_to_excel("I13", 1)
            elif payload in ["brown-egg", "white-egg"]:
                write_to_excel("K17", 1)

            if payload == "white-egg":
                write_to_excel("I17", 1)

            if payload == "brown-egg":
                write_to_excel("J17", 1)

    except Exception as e:
        print(f"Error al procesar el mensaje: {e}")

# Función para obtener la celda correspondiente para el tipo de huevo
def tipo_to_cell(tipo):
    if tipo == "A":
        return "D13"
    elif tipo == "AA":
        return "E13"
    elif tipo == "AAA":
        return "F13"
    else:
        print(f"Tipo de huevo no válido: {tipo}")
        return None

# Configuración del cliente MQTT
mqtt_broker = "192.168.78.18"  # Dirección del broker
mqtt_port = 1883  # Puerto del broker

# Crear un cliente MQTT
client = mqtt.Client()

# Asignar las funciones de conexión y recepción de mensajes
client.on_connect = on_connect
client.on_message = on_message

# Conectar al broker MQTT
client.connect(mqtt_broker, mqtt_port, 60)

# Iniciar el servidor Flask en otro hilo
def start_flask():
    app.run(host='0.0.0.0', port=5000)

flask_thread = threading.Thread(target=start_flask)
flask_thread.start()

# Mantener el cliente MQTT conectado y esperando mensajes
client.loop_forever()

