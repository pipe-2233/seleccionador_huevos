# **Descripción General del Proyecto**

El sistema se basa en un **servidor MQTT**, montado en una **Raspberry Pi Modelo 3**, que actúa como el núcleo central de comunicación. Al servidor están conectados dos microcontroladores **ESP32**, encargados de controlar diferentes partes del proyecto. Además, el servidor interactúa con dos scripts en Python: uno para la captura de imágenes y conexión con un modelo de inteligencia artificial (IA) entrenado, y otro para la gestión de datos en un archivo Excel. La interfaz del sistema se gestiona a través de **Node-RED**, también alojado en la Raspberry Pi.

---

## **Topics Publicados en el Servidor MQTT**

El sistema utiliza los siguientes **topics** para la comunicación entre sus componentes:

- **huevo_en_posicion**: Indica si un huevo está listo para ser procesado.
- **clase_huevo**: Publica el estado del huevo (limpio, sucio, quebrado y color).
- **tipo_huevo**: Clasifica el huevo según su tipo (por ejemplo, A, B, etc.).
- **peso_huevo**: Registra el peso del huevo.
- **Retroalimentacion**: Permite ajustar parámetros de control según los resultados obtenidos.
- **solicitar_excel**: Solicita la subida del archivo Excel actualizado.
- **Setpoint**: Establece valores objetivo para el sistema.
- **Ki, Kp, Kd**: Configuran los parámetros del controlador PID.

---

## **Flujo de Funcionamiento**

### **1. Ingreso del Huevo y Sensado de Distancia**

- El huevo es colocado manualmente en la banda transportadora.
- El sensor ultrasónico mide la distancia.  
  Si esta es menor a **10 cm**, el motor de la banda se detiene.

---

### **2. Captura de Imagen y Análisis**

- El script en Python (`captura_imagenes.py`) toma una foto del huevo.
- La imagen es enviada a un modelo de IA entrenado que predice el estado del huevo (limpio o sucio).
- El resultado se publica en el topic **clase_huevo**.

---

### **3. Clasificación por Limpieza**

- Un **ESP32** suscrito al topic **clase_huevo** decide el siguiente paso:
  - Si el huevo está **sucio**, un servo lo desvía hacia un contenedor específico.
  - Si está **limpio**, el huevo sigue avanzando.

---

### **4. Pesaje y Clasificación por Peso**

- El huevo llega a una **galga extensiométrica** que mide su peso.
- Dependiendo del rango de peso:
  - Se clasifica en categorías usando un servo que controla una rampa de deslizamiento.
  - Otro servo empuja el huevo hacia su destino final.

---

### **5. Gestión de Datos**

- Los resultados de clasificación (tipo y peso) se publican en los topics **tipo_huevo** y **peso_huevo**.
- Otro script en Python procesa estos datos y los registra en un archivo Excel.

---

### **6. Interfaz y Visualización**

- Los datos del sistema se visualizan en tiempo real en **Node-RED**.
- Desde Node-RED, también se puede solicitar el archivo Excel actualizado mediante el topic **solicitar_excel**.

---

## **Infraestructura y Componentes**

### **Hardware**

- **Raspberry Pi Modelo 3** (Servidor MQTT y Node-RED)
- 2 **ESP32** (Controladores de servos y motor)
- **Sensor ultrasónico** (detección de distancia)
- **Motor de banda transportadora**
- **Galga extensiométrica** (pesaje)
- 3 **servos** (clasificación por limpieza y peso)

---

### **Software**

- **Node-RED** (Interfaz gráfica)
- **Scripts en Python** (`captura_imagenes.py`, `procesamiento_excel.py`)
- **Modelo de IA** para clasificación de huevos
- **MQTT** para comunicación entre componentes

---

## **Consideraciones Finales**

Este diseño asegura un procesamiento eficiente y automatizado, desde el ingreso del huevo hasta su clasificación final. Gracias a la integración de **MQTT** y **Node-RED**, el sistema es modular y permite una fácil supervisión y ajustes en tiempo real.
