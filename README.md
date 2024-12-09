# 🥚 **Seleccionador de Huevos**  

¡Bienvenido al proyecto **Seleccionador de Huevos**! 🐣✨ Este sistema combina **Visión Artificial** y tecnologías **IoT** para clasificar huevos y analizar su limpieza en tiempo real. Diseñado para entornos industriales y experimentales, el proyecto utiliza herramientas modernas para optimizar procesos de clasificación.

---

## 🌟 **Características Principales**  

- **🔍 Detección precisa**:  
  Entrenado con un banco de imágenes robusto de la base de datos de **Roboflow**, detecta y clasifica huevos con alta precisión.  

- **🥚 Clasificación avanzada**:  
  Identifica tipos de huevos (limpio, sucio, marrón, etc.) evaluando detalles importantes como su nivel de limpieza.  

- **🚀 Rendimiento optimizado**:  
  Funciona eficientemente en plataformas embebidas como **Raspberry Pi** y puede integrarse en sistemas más grandes.

---

## 🛠️ **Tecnologías y Funcionamiento**  

### 🔗 **IoT y Protocolos**
El proyecto destaca por su integración con tecnologías IoT:  

- **📡 MQTT (Message Queuing Telemetry Transport)**:  
  Permite la comunicación rápida y liviana entre dispositivos.  
  - **Mosquitto**: Broker MQTT configurado para recibir datos del modelo de detección.  
  - Publicación de resultados en temas como `clase_huevo` o `huevo_en_posicion`.  

- **🖥️ Node-RED**:  
  Diseñado para monitorear y controlar el sistema en tiempo real:  
  - Gráficos interactivos que muestran conteos y características de los huevos.  
  - Descarga de resultados en formato **Excel** directamente desde la interfaz.  

### 📉 **Visualización y Almacenamiento**  

- **📊 Grafana**:  
  Visualización de datos históricos mediante gráficos interactivos, alimentados por:  
  - **InfluxDB**: Base de datos diseñada para manejar grandes volúmenes de datos en tiempo real.  

### 📸 **Automatización con Sensores**  

- **Ultrasónico**:  
  Activa la captura de imágenes cuando detecta la presencia de un huevo.  

- **Raspberry Pi**:  
  Procesa la señal del sensor y colabora con el sistema principal para enviar datos al modelo de clasificación.  

---

## 🎯 **Objetivos del Proyecto**  

- **Automatización**: Optimizar el proceso de clasificación de huevos en la industria.  
- **Eficiencia**: Usar recursos ligeros como MQTT para una integración rápida y estable.  
- **Escalabilidad**: Compatible con sistemas embebidos y plataformas industriales más avanzadas.  

---

## 🔧 **¿Cómo se Implementa?**  

1. **Entrenamiento del Modelo**:  
   - Conjunto de datos etiquetados de **Roboflow** con características como "huevo limpio", "sucio", "marrón", etc.  

2. **Integración de Sensores**:  
   - Sensor ultrasónico detecta huevos en movimiento y activa el sistema.  

3. **Procesamiento en Tiempo Real**:  
   - La Raspberry Pi captura la señal, dispara la cámara y envía imágenes al modelo.  
   - Resultados procesados y enviados a plataformas como Node-RED y Grafana.  

4. **Almacenamiento y Visualización**:  
   - Los datos procesados se registran en **InfluxDB** y se visualizan en **Grafana**.  

---

## ✨ **Impacto y Futuro**  

Este proyecto combina **Visión Artificial**, **IoT**, y análisis en tiempo real, ofreciendo una solución escalable para automatizar procesos industriales. En el futuro, puede extenderse para incorporar:  

- Análisis predictivo basado en datos históricos.  
- Integración con otros sensores y plataformas.  
- Mejora del modelo con aprendizaje continuo.  

¡El **Seleccionador de Huevos** marca el inicio de un cambio en la industria, optimizando procesos con innovación y tecnología! 🚀  

---

> **Desarrollado con pasión por automatización e IoT. 🛠️**  
