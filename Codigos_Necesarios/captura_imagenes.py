import paho.mqtt.client as mqtt
import cv2
from datetime import datetime
from roboflow import Roboflow

# Configuración del broker MQTT
BROKER_IP = "192.168.78.18"
BROKER_PORT = 1883
TOPIC = "huevo_en_posicion"
PUBLISH_TOPIC = "clase_huevo"

# URL de la cámara IP
CAMERA_URL = "http://192.168.78.152:4747/video"

# Directorio para guardar las fotos
PHOTO_DIRECTORY = "C:\\Users\\andre\\Downloads\\fotos\\"
# Iniciar la API de Roboflow
rf = Roboflow(api_key="u247o3wH5luRvqN29OnN")
project = rf.workspace().project("eggs14-ub7ik")
model = project.version(1).model

# Función para calcular la nitidez de una imagen
def calculate_sharpness(image):
    """Calcula la nitidez de una imagen usando el gradiente Laplaciano."""
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    laplacian_var = cv2.Laplacian(gray, cv2.CV_64F).var()
    return laplacian_var

# Función para capturar varios fotogramas y seleccionar el más nítido
def capture_best_frame(camera_url, num_frames=5):
    """Captura varios fotogramas y selecciona el más nítido."""
    cap = cv2.VideoCapture(camera_url)
    if not cap.isOpened():
        print("No se pudo abrir la cámara IP.")
        return None

    best_frame = None
    max_sharpness = 0

    for i in range(num_frames):
        ret, frame = cap.read()
        if not ret:
            continue
        
        sharpness = calculate_sharpness(frame)
        if sharpness > max_sharpness:
            max_sharpness = sharpness
            best_frame = frame

    cap.release()
    return best_frame

# Función para guardar la captura y verificar si fue exitosa
def save_frame(frame, filename):
    """Guarda la captura y verifica si fue exitosa."""
    if frame is not None:
        try:
            cv2.imwrite(filename, frame)
            return True
        except Exception as e:
            print(f"Error al guardar la foto: {e}")
            return False
    return False

# Función para realizar la inferencia con el modelo de Roboflow
def make_prediction(filename):
    """Realiza la predicción usando el modelo de Roboflow."""
    prediction = model.predict(filename)
    raw_predictions = prediction.json()['predictions']
    return max(raw_predictions, key=lambda x: x['confidence'], default=None)

# Callback cuando se conecta al broker MQTT
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conexión exitosa al broker MQTT")
        client.subscribe(TOPIC)
        print(f"Suscrito al tema: {TOPIC}")
    else:
        print(f"Error al conectar: Código {rc}")

# Callback cuando se recibe un mensaje del broker
def on_message(client, userdata, msg):
    try:
        value = float(msg.payload.decode())
        print(f"Mensaje recibido en el tema '{msg.topic}': {value}")
        
        if value < 7:
            print("Valor menor a 10, capturando foto...")

            # Capturar el mejor fotograma
            frame = capture_best_frame(CAMERA_URL, num_frames=5)
            if frame is not None:
                timestamp = datetime.now().strftime("%Y-%m-%d_%I-%M-%S %p")
                filename = f"{PHOTO_DIRECTORY}foto_{timestamp}.jpg"
                
                # Guardar la foto y verificar si fue exitosa
                if save_frame(frame, filename):
                    prediction = make_prediction(filename)
                    if prediction:
                        print(f"Predicción seleccionada: {prediction}")

                        # Guardar la predicción en un archivo de texto
                        with open("predictions.txt", "a") as file:
                            file.write(f"Predicción para {filename}:\n")
                            file.write(f"{prediction}\n\n")
                        print("Predicción guardada en predictions.txt.")

                        # Publicar la clase detectada en el tema MQTT
                        client.publish(PUBLISH_TOPIC, prediction['class'])
                        print(f"Clase publicada en '{PUBLISH_TOPIC}': {prediction['class']}")
                    else:
                        print("No se encontraron predicciones válidas.")
                else:
                    print("No se pudo guardar la foto correctamente.")
            else:
                print("No se pudo capturar un fotograma válido.")
    except ValueError:
        print("Error al convertir el mensaje recibido a un número.")
    except Exception as e:
        print(f"Error en el procesamiento del mensaje: {e}")

# Crear instancia del cliente MQTT
client = mqtt.Client()

# Asignar callbacks
client.on_connect = on_connect
client.on_message = on_message

# Conectar al broker
try:
    client.connect(BROKER_IP, BROKER_PORT, 60)
    print(f"Intentando conectar a {BROKER_IP}:{BROKER_PORT}...")
except Exception as e:
    print(f"Error al conectar: {e}")
    exit(1)

# Iniciar el bucle para mantener la conexión y procesar mensajes
client.loop_forever()

