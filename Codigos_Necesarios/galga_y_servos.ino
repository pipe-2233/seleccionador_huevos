#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <HX711.h>

// Pines para la celda de carga
const int DT_PIN = 34;
const int SCK_PIN = 23;

// Configuración de la celda de carga
HX711 scale;

// Crea objeto servo
Servo miServo;
// Configuración de los servos
Servo servo1;
Servo servo2;
Servo servo3;

// Define pines del servo
const int PinServo = 18;
// Pines para los servos
const int SERVO1_PIN = 25;  // Servo para clasificar
const int SERVO2_PIN = 22;  // Servo para empujar
const int SERVO3_PIN = 26;  // Servo pre-empuja

// Configuración del Wi-Fi
const char* ssid = "Prueba";
const char* password = "pipe09nn";

// Configuración del broker MQTT
const char* mqtt_server = "192.168.78.18";  // Dirección IP del broker
const int mqtt_port = 1883;                 // Puerto del broker
const char* topic = "clase_huevo";          // Tema para clases de huevos
const char* peso_topic = "peso_huevo";      // Tema para publicar el peso
const char* tipo_huevo_topic = "tipo_huevo"; // Tema para publicar la clasificación del huevo

// Variables globales
WiFiClient espClient;
PubSubClient client(espClient);

// Función para conectar al Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Wi-Fi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// Callback para manejar los mensajes MQTT recibidos
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en el tema: ");
  Serial.println(topic);

  String mensaje = "";
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  Serial.print("Mensaje: ");
  Serial.println(mensaje);

  // Verificar si el mensaje corresponde a las clases específicas
  if (mensaje == "brow-egg-dirty" || mensaje == "brown-egg-crack" || mensaje == "white-egg-crack" || mensaje == "white-egg-dirty") {
    Serial.println("Clase válida detectada: " + mensaje);
    miServo.write(80);  // Mover al ángulo 120
    delay(7000);
    Serial.println("Volviendo el servo al ángulo 180...");
    miServo.write(150);  // Volver al ángulo 180
  } else {
    Serial.println("Clase no reconocida o mensaje irrelevante.");
  }
}

// Función para reconectar al broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar al broker MQTT...");
    if (client.connect("ESP32Client")) {  // Conexión sin autenticación
      Serial.println("Conectado");
      client.subscribe(topic);
      Serial.print("Suscrito al tema: ");
      Serial.println(topic);
    } else {
      Serial.print("Error (rc=");
      Serial.print(client.state());
      Serial.println("). Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  // Inicializar celda de carga
  Serial.begin(115200);
  Serial.println("Calibrando celda de carga...");
  scale.begin(DT_PIN, SCK_PIN);
  scale.set_scale(975.6);
  scale.tare();

  // Configurar servos
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);

  // Posiciones iniciales
  servo1.write(160);  // Inicial en 90° (neutro)
  servo2.write(180);  // Inicial en 180° (retraído)
  servo3.write(90);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Inicializar el servo
  miServo.attach(PinServo);
  miServo.write(150);  // Posición inicial del servo
}

// Función para mover el servo2 de forma suave
void moveServo2Smooth(int targetAngle, int stepDelay) {
  int currentAngle = servo2.read(); // Leer el ángulo actual del servo
  if (targetAngle > currentAngle) {
    for (int pos = currentAngle; pos <= targetAngle; pos++) {
      servo2.write(pos);
      delay(stepDelay);
    }
  } else if (targetAngle < currentAngle) {
    for (int pos = currentAngle; pos >= targetAngle; pos--) {
      servo2.write(pos);
      delay(stepDelay);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Esperar nuevos mensajes MQTT

  // Leer el peso---------------------------------------------------------
  float peso = scale.get_units(10);
  float peso_efectivo = peso * (-1);  // Invertir signo del peso
  Serial.print("Peso: ");
  Serial.print(peso_efectivo, 1);
  Serial.println(" g");

  // Publicar el peso efectivo en el servidor MQTT
  char peso_str[10];                       // Buffer para almacenar el peso como texto
  dtostrf(peso_efectivo, 6, 2, peso_str);  // Convertir el float a string
  client.publish(peso_topic, peso_str);    // Publicar el peso

  // Clasificación del huevo basada en el peso
  String tipo_huevo = "";

  if (peso_efectivo >= 45.0 && peso_efectivo <= 55.0) {
    tipo_huevo = "A";
    Serial.println("Huevo A detectado: Moviendo servo...");
    servo3.write(110);
    delay(500);
    servo3.write(120);
    delay(700);
    servo1.write(0);  // Mover servo1 a 0°
    moveServo2Smooth(0, 10);  // Mover servo2 a 0° de forma suave con paso de 10 ms
    delay(5000);
    servo1.write(160);  // Regresar servo1 a 160°
    moveServo2Smooth(180, 10);  // Regresar servo2 a 180° de forma suave
    servo3.write(110);
    delay(500);
    servo3.write(100);
    delay(500);
    servo3.write(90);
    delay(1000);
  } else if (peso_efectivo >= 56.0 && peso_efectivo <= 65.0) {
    tipo_huevo = "AA";
    Serial.println("Huevo AA detectado: No se mueve el servo.");
    servo3.write(90);
    delay(700);
    servo1.write(0);  // Mover servo1 a 0°
    moveServo2Smooth(0, 10);  // Mover servo2 a 0° de forma suave con paso de 10 ms
    delay(3000);
    Serial.println("Regresando el servo a 160°...");
    servo1.write(160);  // Regresar servo1 a 160°
    moveServo2Smooth(180, 10);  // Regresar servo2 a 180° de forma suave
    delay(1000);
  } else if (peso_efectivo >= 66.0 && peso_efectivo <= 75.0) {
    tipo_huevo = "AAA";
    Serial.println("Huevo AAA detectado: No se mueve el servo.");
    servo3.write(70);
    delay(500);
    servo3.write(50);
    delay(500);
    servo3.write(45);
    delay(700);
    servo1.write(0);  // Mover servo1 a 0°
    moveServo2Smooth(0, 10);  // Mover servo2 a 0° de forma suave con paso de 10 ms
    delay(5000);
    servo1.write(160);  // Regresar servo1 a 160°
    moveServo2Smooth(180, 10);  // Regresar servo2 a 180° de forma suave
    servo3.write(50);
    delay(500);
    servo3.write(70);
    delay(500);
    servo3.write(90);
    delay(1000);
  } else {
    Serial.println("Peso fuera de rango: No se mueve el servo.");
  }

  // Publicar la clasificación del huevo en MQTT
  client.publish(tipo_huevo_topic, tipo_huevo.c_str());  // Publicar tipo de huevo
}
