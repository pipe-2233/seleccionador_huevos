#include <WiFi.h>
#include <PubSubClient.h>

#include <Arduino.h>

const int sensorPin = 21; // Pin GPIO del sensor HC-020K
volatile unsigned long lastPulseTime = 0; // Tiempo del último pulso detectado
volatile unsigned long pulseInterval = 0; // Intervalo entre pulsos
volatile unsigned long pulseCount = 0; // Número de pulsos detectados

const int pulsesPerRevolution = 1; // Pulsos por revolución

// Configuración del Wi-Fi
const char* ssid = "Prueba";
const char* password = "pipe09nn";

// Configuración del broker MQTT
const char* mqtt_server = "192.168.78.18";  // Dirección IP del broker
const int mqtt_port = 1883;
const char* mqtt_topic_distance = "huevo_en_posicion";
const char* mqtt_topic_setpoint = "Setpoint";
const char* mqtt_topic_rpm = "Retroalimentacion";

// Configuración del sensor ultrasónico
const int trigPin = 13;
const int echoPin = 14;

// Configuración del motor
const int motorPin1 = 5;
const int motorPin2 = 18;
const int motorEnablePin = 19;

// Variables del PID
float Kp = 0.05; // Ganancia proporcional
float Ki = 0.02; // Ganancia integral
float Kd = 0.0; // Ganancia derivativa

float setpoint = 150.0; // Setpoint inicial de RPM
float error = 0; // Error actual
float lastError = 0; // Error en la iteración anterior
float integral = 0; // Integral acumulada
float derivative = 0; // Derivada
int output = 0; // Salida del PID (PWM)

// Cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Función para manejar interrupciones
void IRAM_ATTR handleInterrupt() {
  unsigned long currentTime = micros(); // Tiempo actual en microsegundos
  unsigned long interval = currentTime - lastPulseTime;

  if (interval > 1000) { // Ignorar pulsos con menos de 1000 µs
    pulseInterval = interval;
    lastPulseTime = currentTime;
    pulseCount++;
  }
}

// Función para medir la distancia con el sensor ultrasónico
long measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = (duration / 2) / 29.1;
  return distance;
}

// Función para conectar al Wi-Fi
void setupWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// Función para conectar al broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    if (client.connect("ESP32Motor")) {
      Serial.println("Conectado");
    } else {
      Serial.print("Error de conexión: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorPin), handleInterrupt, FALLING);

  setupWiFi();

  client.setServer(mqtt_server, mqtt_port);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorEnablePin, OUTPUT);

  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);

  Serial.println("Iniciando sistema con control PID...");
}

void loop() {
  noInterrupts();
  float intervalMs = pulseInterval / 1000.0;
  unsigned long pulses = pulseCount;
  pulseCount = 0;
  interrupts();

  float rpm = 0;
  if (intervalMs > 0 && pulses > 0) {
    rpm = (60000.0 / intervalMs) / pulsesPerRevolution * pulses;
  }

  error = setpoint - rpm;
  integral += error;
  derivative = error - lastError;
  lastError = error;

  float output = Kp * error + Ki * integral + Kd * derivative;
  output = constrain(output, 0, 255);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long distance = measureDistance();
  char distanceStr[10];
  snprintf(distanceStr, sizeof(distanceStr), "%ld", distance);
  client.publish(mqtt_topic_distance, distanceStr);

  char setpointStr[10];
  snprintf(setpointStr, sizeof(setpointStr), "%.2f", setpoint);
  client.publish(mqtt_topic_setpoint, setpointStr);

  // Publicar rpm solo si cumple con las condiciones
  if (rpm > setpoint || rpm > setpoint - 100) {
    char rpmStr[10];
    snprintf(rpmStr, sizeof(rpmStr), "%.2f", rpm);
    client.publish(mqtt_topic_rpm, rpmStr);
  }

  if (distance < 10) {
    analogWrite(motorEnablePin, 0);
    delay(6000);
    analogWrite(motorEnablePin, 105);
    delay(500);
    analogWrite(motorEnablePin, output);
  } else {
    analogWrite(motorEnablePin, output);
  }

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    setpoint = input.toFloat();
    Serial.print("Nuevo setpoint: ");
    Serial.println(setpoint);
  }

  delay(300);
}
