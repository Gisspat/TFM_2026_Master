#  DISEÑO E IMPLEMENTACIÓN DE UN SISTEMA IOT PARA LA ESTIMACIÓN DE AFORO EN TRANSPORTE PÚBLICO MEDIANTE SENSORIZACIÓN AMBIENTAL Y VISIÓN POR COMPUTADOR



##  1. Introducción y Propósito del Proyecto

Este proyecto consiste en el diseño y despliegue de una sistema IoT para el ámbito del transporte público inteligente. El objetivo de la propuesta se basa en la digitalización de vehículos urbanos (autobuses) mediante el uso de dispositivos IoT capaces de:
1. **Obtener datos útiles para la estimacion del aforo** en tiempo real (Temperatura, Humedad Relativa y concentración de dióxido de carbono equivalente eco2).
2. **Calcular de forma eficiente el nivel de ocupación** o aforo, para mejorar el sistema de desarrollo de los autobuses .
3. **Transmitir mediante telemetría la estructura de datos obtenidos** mediante protocolos cifrados hacia un gateway servidor para su posterior análisis.



##  2. Herramientas Tecnológicas y Ecosistema de Versiones Empleadas

Para asegurar la estabilidad, fiabilidad y seguridad del sistema, se han empleado un conjunto de herramientas estándar, que se muestran en  el siguiente esquema de versiones:

* **Entorno de Desarrollo Principal:** Visual Studio Code (VS Code) con la extensión oficial del fabricante.
* **Framework de Programación Embebida:** `ESP-IDF` (Espressif IoT Development Framework), en la que para este proyecto se ha usado la version 5.5.1.
* **Sistema Operativo en Tiempo Real:** `FreeRTOS` (integrado nativamente en el núcleo del firmware).
* **Ecosistema de Backend:** `Python` para la lógica del servidor de persistencia y pasarela de datos, obtenidos mediante el uso de la version 3.11.2.
* **Protocolo de red ligero:** MQTT / MQTTS (Message Queuing Telemetry Transport sobre TLS).
* **Gestión de Dependencias y Formato de Datos:** Librería interna `cJSON` para el parseo eficiente de estructuras de texto plano dentro del microcontrolador.

### Paso 3.: Arquitectura del Proyecto en VS Code  
En la siguiente imagen se muestra la estructura usada del proyecto dentro del entorno de desarrollo Visual Studio Code.  
Cada componente del sistema (sensores, Wi‑Fi, MQTT, parser de ubicación y lógica principal) se encuentra encapsulado en diferentes carpetas para asi obtener una estructura modular, lo que facilita la mantenibilidad, escalabilidad y depuración del proyecto.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/aeb349323fa2146ea3b0863abdf7941f0a41b334/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20190127.png)


### 4.– Configuración del Proyecto en *menuconfig*

#### 4.1 – Configuración del Serial Flasher
Esta captura muestra los parámetros del módulo *Serial Flasher*, donde se define el modo SPI, la velocidad de escritura, el tamaño de la memoria flash y el comportamiento del reinicio antes y después del flasheo. Estos ajustes aseguran una programación estable y compatible con el ESP32‑S3.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/79f24ca3ec63eda5ede31962dd865ba4a8dac882/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-05%20001602.png)



#### 4.2 – Configuración de la Tabla de Particiones
En esta sección se selecciona la tabla de particiones del firmware. Para este proyecto se utiliza *Single factory app, no OTA*, que reserva la memoria para una única aplicación sin actualizaciones OTA. También se muestra el offset de la tabla y la generación del checksum MD5 para verificar su integridad.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/79f24ca3ec63eda5ede31962dd865ba4a8dac882/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-05%20001616.png)



#### 4.3– Configuración del Cliente MQTT
Esta captura corresponde a la configuración del componente MQTT. Se habilitan los protocolos MQTT 3.1.1 y las variantes seguras mediante SSL y WebSocket Secure, permitiendo comunicaciones cifradas con el broker y garantizando seguridad en la transmisión de datos.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/79f24ca3ec63eda5ede31962dd865ba4a8dac882/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-05%20001642.png)




## 4.: Pasos Seguidos en el Desarrollo Tecnológico  
A continuación, se detallan de forma cronológica los pasos realizados para la construcción del ecosistema IoT completo basado en ESP32, sensores ambientales, geolocalización móvil y mensajería MQTT.

### Paso 4.1: Integración del Hardware y Sensores Ambientales  
Se ensambló el microcontrolador **ESP32‑WROOM** junto con los sensores SI7021 (temperatura/humedad) y SGP30 (eCO₂/TVOC).  
Se implementaron los controladores I²C dentro del framework ESP‑IDF, aplicando las funciones de transferencia oficiales de cada fabricante para garantizar la fiabilidad de las mediciones.

### Paso 4.2: Configuración del Módulo Wi‑Fi y Máquina de Estados  
Se inicializó el ESP32 en modo estación (STA), registrando manejadores de eventos para:  
- Inicio del módulo Wi‑Fi.  
- Reconexión automática en caso de pérdida.  
- Obtención de dirección IP.  

Tras la asignación de IP, se activó el cliente MQTT para iniciar la telemetría segura mediante TLS.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/eb4a432067e9ff2e2951734ed1b6057eab64a7e2/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20165921.png)

### Paso 4.3: Escaneo Activo de Redes Wi‑Fi y Ubicación Relativa  
Se añadió un proceso de escaneo periódico de redes Wi‑Fi cercanas.  
El sistema filtra los puntos de acceso por RSSI para:  
- Contar cuántas redes están dentro del rango operativo.  
- Identificar el punto de acceso más cercano.  

Esta información se utiliza como indicador de **ubicación relativa del autobús** cuando no se dispone de GPS.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185126.png)


### Paso 4.4: Procesamiento de Umbrales Ambientales y Estimación de Aforo  
Se definieron umbrales cualitativos para temperatura, humedad y eCO₂.  
El nivel de eCO₂ se empleó como métrica directa para estimar el **estado de aforo**:  
- AFORO BAJO  
- AFORO MEDIO  
- AFORO ALTO  

Este enfoque permite inferir ocupación sin cámaras ni sensores invasivos.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185203.png)


### Paso 4.5: Integración de Geolocalización mediante OwnTracks  
Se configuró la aplicación **OwnTracks** en un dispositivo móvil ubicado dentro del autobús.  
La app envía latitud y longitud al broker MQTT.  
El ESP32 se suscribe al tópico correspondiente y extrae las coordenadas mediante la librería `cJSON`.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185338.png)


### Paso 4.6: Publicación de Telemetría en HiveMQ (MQTTS)  
El ESP32 publica periódicamente un mensaje JSON con:  
- lat, lon  
- temp, hum  
- eCO₂  
- estado de aforo  
- redes detectadas  
- RSSI  



![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185511.png)


La comunicación se realiza mediante **MQTTS**, utilizando un certificado CA almacenado en el firmware para garantizar la seguridad.

### Paso 4.7: Recepción de Datos en el PC y Generación Automática del CSV  
En el ordenador se ejecuta el script Python `mqtt_to_csv.py`, encargado de:  
- Suscribirse al tópico MQTT.  
- Recibir y mostrar los datos en la terminal.  
- Crear el archivo `dataset.csv` con cabecera si no existe.  
- Añadir cada nueva lectura en tiempo real.  

El CSV final contiene:  
- Timestamp  
- Latitud y longitud  
- Temperatura, humedad, eCO₂  
- Estado de aforo  
- Redes detectadas  
- RSSI

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/72a767050a732abc16d736020c9f6a6706e03edc/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20184727.png)



En esta captura se muestra la rutina de procesamiento de mensajes recibidos desde el broker. El callback decodifica el payload, extrae los parámetros de telemetría (ubicación, variables ambientales, aforo y métricas de red), genera una marca temporal y almacena toda la información en un archivo CSV para su posterior análisis.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20184829.png)

En esta captura se muestra la rutina completa de inicialización del Wi‑Fi en modo estación (STA). Aquí se preparan las interfaces de red, se cargan los parámetros por defecto del driver, se registran los manejadores de eventos del sistema y finalmente se inyectan las credenciales antes de arrancar el controlador inalámbrico.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185534.png)


### Paso 4.8: Validación del Sistema Completo  
Se verificó el funcionamiento extremo a extremo:  
**Sensores → ESP32 → HiveMQ → PC → CSV**.  
El sistema demostró estabilidad, reconexión automática y consistencia en la telemetría generada.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185700.png)

### 5.: Ejecución del archivo .py para la obtención del dataset tabular
En este apartado se muestra el lugar y código que se ejecuta en la terminal para que reciba la estrucutra de datos procedente de visual por MQTT.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/317a2b44362407e6ccde68bd493a3574054d173d/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-04-08%20233859.png)

### 6.: Arquitectura de Clasificación  
En esta etapa se implementa la lógica encargada de transformar los valores numéricos obtenidos por los sensores en categorías cualitativas.  
Esta arquitectura de clasificación permite interpretar el estado ambiental del autobús mediante niveles de temperatura, humedad y CO₂, y derivar de ellos el estado de aforo estimado (BAJO, MEDIO o ALTO).


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185311.png)


### 7.: Arquitectura de Lectura del Sensor SGP30  
Este módulo implementa la comunicación I²C con el sensor SGP30 para obtener mediciones de calidad del aire, incluyendo los valores de eCO₂ y TVOC.  
El proceso abarca el disparo de la solicitud de medición, el tiempo de espera requerido por el sensor, la extracción de la trama de datos y el control de excepciones en caso de errores de lectura.  
Estos valores son fundamentales para estimar el nivel de ocupación del autobús y evaluar la calidad del aire interior.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185751.png)

### 8.: Arquitectura de Envío de Telemetría mediante MQTT  
Este módulo se encarga de construir el mensaje JSON que contiene toda la telemetría generada por el sistema: ubicación, variables ambientales, estado de aforo, redes detectadas y nivel de señal.  
Una vez formateado, el mensaje se publica en el tópico `bus/data` utilizando el cliente MQTT seguro previamente inicializado.  
Este componente constituye el núcleo del flujo de datos hacia la nube.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185853.png)


### 9.: Arquitectura del Manejador de Eventos MQTT  
Este módulo gestiona los eventos generados por el cliente MQTT.  
Incluye la detección de la conexión exitosa al broker, la suscripción al tópico de geolocalización enviado por OwnTracks y el procesamiento de los mensajes entrantes mediante la función `parse_location()`.  
Esta arquitectura permite integrar la ubicación del autobús dentro del flujo de telemetría general del sistema.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185929.png)

### 10.: Certificado empleado
Este apartado muestra el certificado empleado para la posible comunicación por MQTT con HiveMQ, y con ello poder realizar el envió de la estructura de datos.
En la imagen se muestra la estructura completa del certificado ya que sin ella no se podria enviar los datos.


![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20190010.png)



### 11.: Arquitectura de Inicio del Cliente MQTT  
Este módulo inicializa el cliente MQTT seguro utilizado por el sistema.  
Incluye la configuración del broker remoto, la verificación mediante certificado, la inyección de credenciales de acceso y el registro del manejador de eventos MQTT.  
Finalmente, se activa el cliente, habilitando el envío y recepción de telemetría en la nube.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20185949.png)



### 12.: Arquitectura del Módulo de Parsing de Ubicación  
Este módulo procesa los mensajes JSON recibidos desde OwnTracks a través del broker MQTT.  
El sistema construye la estructura cJSON desde el payload crudo, extrae los parámetros de latitud y longitud, valida los campos y actualiza la memoria global del sistema.  
Finalmente, libera la memoria dinámica utilizada, garantizando un manejo seguro y eficiente de los recursos.

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/bb257f1d419a622223ae70470163b95449edbe25/Im%C3%A1genes_TFM/Captura%20de%20pantalla%202026-06-03%20190101.png)


## 13.: Conclusiones Finales del Proyecto  

1. **Integración IoT completa y funcional**  
   Se logró un sistema capaz de monitorizar en tiempo real condiciones ambientales, ocupación estimada y ubicación del autobús mediante tecnologías abiertas y hardware de bajo coste.

2. **Arquitectura modular y escalable**  
   La separación en componentes (sensores, Wi‑Fi, MQTT, parser, Python) permite extender el sistema a nuevos sensores, nuevos vehículos o nuevas funcionalidades sin modificar la estructura base.

3. **Estimación de aforo basada en eCO₂**  
   El uso del eCO₂ como indicador indirecto de ocupación resultó eficaz, económico y no invasivo, demostrando su utilidad en entornos de transporte público.

4. **Generación automática de dataset estructurado**  
   El sistema produce un archivo CSV listo para análisis, visualización o entrenamiento de modelos predictivos, facilitando futuras fases de investigación.

5. **Aplicabilidad real en escenarios Smart City**  
   La solución demuestra que es viable monitorizar un autobús en tiempo real utilizando únicamente un ESP32, sensores económicos, un móvil con OwnTracks y un broker MQTT en la nube.

6. **Base sólida para trabajos futuros**  
   El proyecto deja preparado el camino para:  
   - dashboards en tiempo real,  
   - predicción de aforo,  
   - optimización de rutas,  
   - integración con plataformas Smart City.
