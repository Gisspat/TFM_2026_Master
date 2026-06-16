# DISEÑO E IMPLEMENTACIÓN DE UN SISTEMA IOT PARA LA ESTIMACIÓN DE AFORO EN TRANSPORTE PÚBLICO MEDIANTE SENSORIZACIÓN AMBIENTAL Y VISIÓN POR COMPUTADOR

# Monitoreo de Pasajeros - Módulo de Captura ESP32-S3 EYE y Servidor YOLOv8

Este sección del TFM se encarga de la captura de imágenes en tiempo real utilizando el hardware de la cámara integrada del **ESP32-S3 EYE** y su transmisión inalámbrica hacia un servidor local que ejecuta un modelo de inteligencia artificial **YOLOv8** para el conteo automatizado de personas.



## 1. Introducción

El objetivo principal de este módulo es actuar como el "ojo" del sistema de conteo de pasajeros en el autobús inteligente. Utilizando el microcontrolador ESP32-S3 EYE, el sistema inicializa el sensor de la cámara, establece una conexión WiFi estable y transmite los frames capturados mediante peticiones HTTP POST hacia una API REST. El servidor procesa el flujo de imágenes en tiempo real y devuelve el número total de personas detectadas basándose en cajas delimitadoras (*bounding boxes*).


## 2. Herramientas Empleadas

Para el desarrollo e implementación de este módulo, se ha seleccionado un conjunto de tecnologías robustas y de alto rendimiento en el ámbito de IoT e Inteligencia Artificial:

* **ESP-IDF (v5.5.1):** Framework de desarrollo oficial de Espressif empleado para el microcontrolador ESP32-S3 y de igual manera siendo compatible con las fucniones a cumplir por el mismo, utilizado para programar el hardware de manera eficiente.
* **Python (3.11.2):** Lenguaje de programación en el entorno del servidor para el desarrollo del modelo de IA y la API.
* **FastAPI:** Framework web de alto rendimiento empleado para levantar los endpoints encargados de recibir el flujo de datos multimedia.
* **Uvicorn:** Servidor ASGI ultrarrápido utilizado para desplegar la aplicación FastAPI de manera local.
* **Ultralytics YOLOv8:** Modelo de visión artificial de última generación optimizado para tareas de detección de objetos en tiempo real (clase person).
* **Visual Studio Code:** Entorno de desarrollo integrado (IDE) para la edición del código, depuración y control de versiones con Git.



## 3. Arquitectura y Estructura del Proyecto

El flujo de datos del sistema sigue un modelo Cliente-Servidor optimizado para entornos distribuidos de IoT, minimizando la carga computacional en el nodo del autobús:

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/2216d24c6ee3b5b1536a4e40c5e4e83431ac7a43/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20134126.png)

### 4. Componentes Clave:
* **Cliente (Hardware):** Microcontrolador ESP32-S3 EYE programado sobre la estructura nativa de **ESP-IDF**. Gestiona los buffers de la cámara situados en la memoria PSRAM para no saturar la memoria interna del chip.
* **Servidor (IA):** API desarrollada con **FastAPI / Uvicorn** que recibe los bytes crudos de la imagen, los decodifica e implementa la detección de la clase person (ID 0) a través de la librería Ultralytics (YOLOv8).



## 5. Configuración del Entorno (idf.py menuconfig)

Para garantizar la estabilidad del streaming de video en tiempo real, la correcta gestión de la memoria RAM interna y externa, y la asignación adecuada de los identificadores de hardware, se aplicaron las siguientes configuraciones dentro del apartado de configuración nativa de ESP-IDF:

### A. Soporte de Memoria Externa (PSRAM)
Dado que las imágenes procesadas ocupan un volumen considerable de datos, se habilita el soporte para la memoria RAM externa conectada por SPI (PSRAM). Esto evita desbordamientos en la memoria interna del chip durante la asignación de buffers de video:
* **Ruta:** `Component config` ➔ `ESP PSRAM`
![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/a17a2d5325254ad3ee3b64475e90079385bb3793/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20132343.png)

Dentro de la configuración avanzada de la PSRAM, se habilita el uso de **Octal Mode PSRAM** y se establece la velocidad del reloj a **80MHz** para maximizar la tasa de transferencia de datos:
* **Ruta:** `Component config` ➔ `ESP PSRAM` ➔ `SPI RAM config`
![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/a17a2d5325254ad3ee3b64475e90079385bb3793/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20132404.png)

### B. Selección y Compatibilidad de los Sensores de Cámara
Se configuran los drivers multimedia para dar soporte a los diferentes sensores del ecosistema Espressif, garantizando la compatibilidad con el sensor **OV2640** integrado en la placa ESP32-S3 Eye:
* **Ruta:** `Component config` ➔ `Camera configuration`
![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/a17a2d5325254ad3ee3b64475e90079385bb3793/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20132646.png)

### C. Configuración de la Tabla de Particiones (Partition Table)
Se establece el uso de una tabla de particiones personalizada (`Custom partition table CSV`) basada en el archivo local `partitions.csv`. Esto es indispensable para organizar adecuadamente el tamaño asignado a la aplicación, los datos de calibración y el almacenamiento intermedio:
* **Ruta:** `Partition Table`
![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/a17a2d5325254ad3ee3b64475e90079385bb3793/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20132618.png)

Como se observa en la siguiente imagen, en la tabla de particiones personalizada establecida en el apartado de (`factory`) se ha dimensionado de manera óptima a **8000K** (aproximadamente 8 MB) para dar soporte al tamaño completo del firmware integrado:

![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/67ae45f8cf466f9ee1a71194f3b09106d22e4e55/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20140116.png)

### D. Optimización de la Pila de Red Wi-Fi
Para obtener una mejora de la latencia en el envío de las imágenes, se incrementan los buffers estáticos y dinámicos de transmisión y recepción, y se activan los mecanismos de agregación de tramas **Wi-Fi AMPDU TX** y **Wi-Fi AMPDU RX**, junto con la optimización de velocidad en la memoria IRAM:
* **Ruta:** `Component config` ➔ `Wi-Fi`
![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/a17a2d5325254ad3ee3b64475e90079385bb3793/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20132716.png)

### E. Configuración del Cliente HTTP y Seguridad
Se habilita el componente del cliente HTTP integrado de Espressif con soporte para conexiones seguras (**Enable https**), permitiendo que el módulo IoT realice peticiones de tipo `POST` de forma fiable hacia servidores o APIs externas:
* **Ruta:** `Component config` ➔ `ESP HTTP client`
![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/a17a2d5325254ad3ee3b64475e90079385bb3793/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20132529.png)

### F. Parámetros de Grabación del Chip (Serial Flasher)
Se ajusta la velocidad de comunicación del puerto serie para la carga del firmware, seleccionando una velocidad de bus de **80 MHz** y un tamaño máximo de memoria flash asignada de **8 MB**, adaptándose a las dimensiones físicas reales del chip utilizado:
* **Ruta:** `Serial flasher config`
![image alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/a17a2d5325254ad3ee3b64475e90079385bb3793/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20132319.png)


#### A. Inicialización Dinámica del Stack Wi-Fi
Para asegurar una transmisión inalámbrica directa e inmediata hacia la pasarela FastAPI, se implementó un flujo dedicado de configuración de radio a bajo nivel mediante la función `wifi_init_manual()`:



Este bloque valida los siguientes puntos de control de ingeniería:
* **`esp_netif_init()`**: Inicializa la pila TCP/IP nativa de Espressif.
* **`esp_wifi_init()`**: Aloja en memoria los buffers del driver de radio asignando los parámetros estructurales por defecto.
* **`strcpy(...)`**: Inyecta en caliente las credenciales de la red inalámbrica (`Livebox6-7ED0-24G`) para forzar una petición de asociación en modo estación (`WIFI_MODE_STA`).

#### B. Flujo Secuencial del Ciclo de Vida del Dispositivo
El arranque y la persistencia de las tareas en tiempo real se coordinan de forma unificada desde la subrutina del sistema `app_main()`:

## 6. Arquitectura del Software, Estructuras de Datos e Inicialización del Nodo IoT
**Archivo:** `main/app_main.cpp`

### 6.1 Inicialización Manual del Stack Wi‑Fi

A continuación se muestra la captura correspondiente a la función `wifi_init_manual()`, donde se realiza:

- **Inicialización del stack TCP/IP** (`esp_netif_init()`).
- **Creación del event loop** del sistema.
- **Configuración del driver Wi‑Fi** con parámetros por defecto.
- **Introducción de los datos del SSID y contraseña**.
- **Arranque del modo estación** y solicitud de conexión.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/f0480aaa5083b564bf2490599190e5c75413657d/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20142101.png)

### 6.2 Ciclo de Vida del Firmware y Arranque del Motor de IA

La siguiente captura corresponde al flujo principal del firmware (`app_main()`), donde se coordinan los procesos críticos del nodo IoT:

- **Inicialización del sistema NVS**, necesaria para gestionar claves, calibraciones y parámetros persistentes.
- **Arranque del módulo Wi‑Fi** mediante la función `wifi_init_manual()`, que habilita la conectividad previa al envío de frames.
- **Ajuste de prioridades del sistema operativo FreeRTOS**, garantizando que las tareas de captura e inferencia mantengan la latencia mínima.
- **Creación y ejecución del pipeline de inferencia**, enlazando el motor de IA con el flujo de frames capturados.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/f0480aaa5083b564bf2490599190e5c75413657d/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20142139.png)

**Archivo:** `main/frame_cap_pipeline.cpp`
### 6.3 Pipeline de Captura DVP para ESP32‑S3  

En este fragmento se implementa el pipeline de captura de vídeo para arquitecturas **ESP32‑S3** utilizando bus **DVP**.  
La función `get_lcd_dvp_frame_cap_pipeline()` realiza tres tareas clave:

- **Cálculo del tamaño de frame** en función de la resolución de la pantalla LCD.  
- **Instanciación del objeto de cámara `WhoS3Cam`**, ajustando el número de buffers y el tiempo de procesamiento (`MODEL_TIME + 3`).  
- **Creación del objeto `WhoFrameCap`** e inyección del nodo de captura (`WhoFetchNode`) en el flujo principal de procesamiento.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/426ffd9c003dabc90879ee96a782efbfc13e15b5/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224126.png)

**Archivo:** `main/idf_component.yml`
### 6.4 Declaración de Dependencias del Componente  

Este archivo define las **dependencias externas** necesarias para que el módulo de detección funcione correctamente dentro del entorno ESP‑IDF.  
En este caso se declaran:

- **`esp32_s3_eye_noglib`** → Paquete que habilita compatibilidad con el hardware ESP32‑S3 EYE sin librería gráfica.  
- **`pedestrian_detect`** → Modelo de inferencia para detección de personas integrado como componente externo.

Estas dependencias permiten que el firmware cargue automáticamente los controladores y modelos requeridos durante la compilación.


![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/426ffd9c003dabc90879ee96a782efbfc13e15b5/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224339.png)


**Archivo:** `main/CMakelists.txt`
### 6.5 Registro del Componente y Declaración de Dependencias  

Este archivo define la configuración de compilación del componente principal del sistema de detección.  
En él se registran:

- **Directorios fuente e include** utilizados por el módulo.  
- **Dependencias obligatorias** del firmware, incluyendo Wi‑Fi, NVS, eventos, red TCP/IP y cliente HTTP.  
- **Modelo de inferencia seleccionado dinámicamente**, mediante la variable `${DETECT_MODEL}`, que permite enlazar el modelo YOLO correspondiente como dependencia opcional.

Este mecanismo garantiza que el sistema cargue automáticamente los módulos de detección y los controladores necesarios durante la fase de construcción del firmware.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224416.png)

**Archivo:** `components/who_app/who_detect_app/who_detect_app_lcd.cpp`
### 6.6 Inicialización del Módulo de Detección con Salida LCD  

Este constructor configura el módulo encargado de visualizar en pantalla los resultados del motor de detección.  
El flujo de inicialización incluye:

1. **Asignación del nodo de captura de vídeo**, tomando el último nodo del pipeline si no se especifica uno explícito.  
2. **Inicialización del dispositivo LCD**, creación de la tarea asociada y registro del callback de refresco (`lcd_disp_cb`).  
3. **Configuración del canvas gráfico**, utilizado para dibujar las *bounding boxes* generadas por el modelo YOLO.  
4. **Registro de callbacks del motor de inferencia**, incluyendo el callback de resultados y la rutina de limpieza de eventos.


![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224536.png)

### 6.7 Ajuste de Escalado PPA, Creación de la Cola de Frames y Lanzamiento del Hilo HTTP  

Este bloque implementa tres elementos críticos del flujo de procesamiento:

1. **Ajuste dinámico de escalado (PPA)**  
   Cuando el hardware soporta PPA, se recalculan los factores de escala (`rescale_x`, `rescale_y`) comparando las dimensiones del frame del LCD con las del nodo de detección.  
   Esto garantiza que el motor de inferencia reciba imágenes correctamente proporcionadas.

2. **Creación de la cola de intercambio entre hilos**  
   Se inicializa `frame_queue` con capacidad para **dos imágenes**, actuando como búfer intermedio entre el pipeline de captura y el hilo de transmisión HTTP.

3. **Lanzamiento del hilo dedicado a envíos HTTP**  
   Se crea una tarea independiente (`http_sender_task`) fijada en **CPU1**, encargada exclusivamente de enviar los frames al servidor FastAPI sin bloquear el pipeline principal.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224612.png)

### 6.8 Ejecución del Pipeline y Callbacks de Detección  

Este bloque implementa la ejecución asíncrona de todos los nodos del pipeline de captura y visualización, así como los callbacks que conectan los resultados del modelo YOLO con la pantalla LCD:

1. **Ejecución asíncrona del pipeline (`run()`)**  
   - Se activa el scheduler interno (`WhoYield2Idle`).  
   - Se ejecutan todos los nodos del pipeline con sus respectivos tamaños de pila.  
   - Se lanza el refresco del LCD y el motor de detección en paralelo.

2. **Callback de resultados de detección (`detect_result_cb`)**  
   - Recibe las *bounding boxes* generadas por YOLO.  
   - Las almacena en el módulo gráfico para su posterior renderizado en pantalla.

3. **Callback de refresco LCD (`lcd_disp_cb`)**  
   - Filtra y procesa el frame recibido.  
   - Incrementa un contador interno para controlar la frecuencia de envío.  
   - Cada 50 frames prepara un buffer JPEG para transmisión.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224642.png)

### 6.9  HTTP: Espera de IP, Procesamiento de Cola y Envío de Frames  

Este bloque implementa el hilo dedicado exclusivamente a la transmisión de imágenes hacia el servidor FastAPI.  
Su funcionamiento se divide en dos fases principales:

1. **Bloqueo inicial hasta obtener una IP válida**  
   El hilo no comienza a consumir la cola de frames hasta que el ESP32‑S3 obtiene una dirección IP estable.  
   Esto evita pérdidas de datos y garantiza que el primer envío ocurra únicamente cuando la red está completamente operativa.

2. **Bucle infinito de escucha de la cola de transmisión**  
   - El hilo permanece dormido hasta que llega un nuevo frame (`xQueueReceive`).  
   - Cada imagen recibida se envía al backend Python mediante `send_frame_to_server()`.  
   - Tras el envío, se libera la memoria asociada al buffer para evitar fugas.  
   - Se registran logs informativos para depuración en tiempo real.

Este diseño asegura un uso eficiente de CPU y memoria, manteniendo la transmisión estable incluso bajo carga.


![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224721.png)

### 6.10 Cliente HTTP – Construcción del Paquete POST para Envío de Imágenes  

Este bloque implementa la fase inicial del cliente HTTP encargado de enviar cada frame JPEG al servidor FastAPI.  
La función `send_frame_to_server()` realiza los siguientes pasos:

1. **Validación del buffer de imagen**  
   Se descarta la operación si el puntero es nulo o el tamaño es cero, evitando fallos de transmisión.

2. **Configuración del cliente HTTP (`esp_http_client_config_t`)**  
   - Definición de la URL del endpoint `/detect`.  
   - Selección del método `POST`.  
   - Aumento del `timeout` a 5 segundos para soportar imágenes grandes.  
   - Ajuste del tamaño del buffer interno del cliente (4 KB).

Esta configuración prepara el entorno de red para la construcción del paquete POST y el posterior envío del frame al backend Python.


![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224750.png)

### 6.11 Cliente HTTP – Envío del Frame JPEG, Gestión de Respuesta y Liberación de Recursos  

Este bloque implementa la fase final del proceso de transmisión de imágenes hacia el servidor FastAPI.  
Incluye la construcción del paquete POST, el envío del buffer JPEG y la evaluación de la respuesta del backend:

1. **Establecimiento de encabezados HTTP**  
   - `Content-Type: image/jpeg` para indicar el formato.  
   - `Content-Length` calculado dinámicamente según el tamaño del buffer (`jpg_len`).  
   Estos encabezados son esenciales para que FastAPI pueda interpretar correctamente el cuerpo de la petición.

2. **Asignación del cuerpo del POST**  
   El buffer JPEG se pasa directamente mediante `esp_http_client_set_post_field()`, evitando copias innecesarias.

3. **Ejecución del POST y evaluación de la respuesta**  
   - Se ejecuta la petición con `esp_http_client_perform()`.  
   - Si el servidor responde correctamente, se obtiene el código HTTP y el mensaje devuelto.  
   - En caso de error, se registra el motivo mediante `esp_err_to_name(err)`.

4. **Limpieza del cliente HTTP**  
   Se destruye el cliente y se liberan los sockets TCP asociados, garantizando que no queden recursos abiertos.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20224809.png)


**Archivo:** `servidor_yolo.py`

### 6.12 Inicialización del Servidor de Inferencia YOLOv8  

Este bloque corresponde a la inicialización del servidor de inteligencia artificial encargado de procesar los frames enviados por el ESP32‑S3.  
Incluye:

1. **Carga del modelo YOLOv8**  
   Se importa la clase `YOLO` de Ultralytics y se carga el modelo entrenado (`best.pt`), que será utilizado para la detección de personas.

2. **Inicialización del servidor FastAPI**  
   Se crea la instancia principal de la aplicación que gestionará los endpoints de inferencia.

3. **Creación del directorio de almacenamiento**  
   Se genera la carpeta `capturas_esp32` para guardar los frames anotados, permitiendo verificar visualmente los resultados del modelo.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20225306.png)

### 6.13 Endpoint `/detect` – Recepción del Frame, Decodificación y Ejecución de YOLOv8  

Este bloque implementa el endpoint principal del servidor FastAPI encargado de procesar cada imagen enviada por el ESP32‑S3.  
El flujo completo incluye:

1. **Lectura del cuerpo de la petición**  
   Se reciben los bytes del frame JPEG enviados por el microcontrolador y se registra su tamaño para depuración.

2. **Validación del buffer recibido**  
   - Si el tamaño es inferior a 100 bytes, se considera un buffer vacío.  
   - Si la decodificación falla, se clasifica como JPEG corrupto.

3. **Conversión y decodificación de la imagen**  
   - Los bytes se convierten en un array de NumPy.  
   - Se decodifica la imagen con OpenCV (`cv2.imdecode`).

4. **Ejecución del modelo YOLOv8**  
   - Se procesa la imagen con el modelo cargado.  
   - Se obtiene el número de cajas detectadas (personas).

5. **Guardado del último frame anotado**  
   Se genera una imagen con las detecciones dibujadas y se almacena en `capturas_esp32/ultimo_frame.jpg`.

6. **Respuesta al cliente**  
   Se devuelve un diccionario JSON con el número de personas detectadas.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/3a6821915ea0f0043d63261a7af9e327884c2717/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20225318.png)



**Archivo:** `components/who_app/who_cam/who_s3_cam.cpp`

### 6.14 Configuración e Inicialización del Sensor de Cámara (Clase `WhoS3Cam`)  

Este constructor implementa la inicialización completa del sensor de cámara utilizado por el ESP32‑S3.  
El proceso incluye:

1. **Inicialización del bus I2C**  
   Se habilita la comunicación con el sensor mediante `bsp_i2c_init()`, requisito previo para configurar el OV2640.

2. **Construcción de la estructura de configuración del sensor**  
   A partir de `BSP_CAMERA_DEFAULT_CONFIG`, se ajustan:
   - Formato de píxel (`pixel_format`)  
   - Resolución (`frame_size`)  
   - Número de frame buffers (`fb_count`)  
   - Modo de captura (`CAMERA_GRAB_LATEST`)  
   - Frecuencia del reloj XCLK (20 MHz si se usa JPEG)

3. **Inicialización del driver de cámara**  
   Se ejecuta `esp_camera_init()` con la configuración final, activando el sensor.

4. **Activación del modo PSRAM**  
   Permite almacenar los frame buffers en memoria externa, evitando saturación de RAM interna.

5. **Aplicación de flips vertical y horizontal**  
   Se ajusta la orientación de la imagen según los parámetros del constructor, garantizando que la imagen llegue correctamente orientada al pipeline.


![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/6a572283086628baa4e38e4408f773a248f9baa4/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20233224.png)

### 6.15 Gestión de Frame Buffers y Control de Orientación del Sensor  

Este bloque implementa las funciones auxiliares de la clase `WhoS3Cam`, responsables de la gestión de los frame buffers y del control de orientación del sensor OV2640:

1. **Destructor del driver de cámara**  
   Libera correctamente los recursos asociados al sensor mediante `esp_camera_deinit()`, garantizando un apagado limpio del módulo.

2. **Obtención de un frame buffer (`cam_fb_get`)**  
   - Solicita un frame al driver (`esp_camera_fb_get()`).  
   - Selecciona el índice de buffer interno.  
   - Copia el contenido en la estructura `cam_fb_t` gestionada por la clase.  
   - Devuelve un puntero al frame listo para procesamiento.

3. **Retorno del frame buffer (`cam_fb_return`)**  
   Devuelve el buffer al driver para que pueda ser reutilizado, evitando fugas de memoria y saturación del pool de buffers.

4. **Aplicación de flips verticales y horizontales (`set_flip`)**  
   - Si no se requiere ningún flip, la función retorna inmediatamente.  
   - Obtiene el sensor mediante `esp_camera_sensor_get()`.  
   - Aplica el volteo vertical u horizontal según corresponda.  
   - En caso de error, registra un mensaje mediante `ESP_LOGE`.

Este módulo asegura una gestión eficiente de los frames capturados y permite ajustar la orientación de la imagen según la disposición física del sensor en la placa.

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/6a572283086628baa4e38e4408f773a248f9baa4/TFM_Imagenes/Captura%20de%20pantalla%202026-06-06%20233307.png)


## 7. Proceso de Ejecución y Detección de Personas

Para poner en marcha todo el sistema de análisis visual, se siguieron los siguientes pasos:

### Paso 1: Conexión Física del Dispositivo
Para empezar se conecta el dispositivo **ESP32-S3 EYE** al PC mediante un cable de datos USB Tipo C para garantizar la alimentación de la cámara y la interfaz serie.

### Paso 2: Levantar el Servidor de Inteligencia Artificial (YOLO)
En el PC, dentro de la ruta del script del servidor, se inicia el servicio con Uvicorn ejecutando el siguiente comando en la terminal, para la hacer posible la recepción de datos procedentes del ESP32 S3 EYE, para poder establecer el conteo automático de personas:

python -m uvicorn servidor_yolo:app --host 0.0.0.0 --port 8000

### Paso 3: Compilación y Monitoreo del ESP32-S3 Eye
A continuación se construye, flashea el firmware y se abre el monitor serie para observar la inicialización del hardware IoT:

idf.py build flash monitor

### Paso 4: Flujo Automático en Tiempo Real
1. El módulo activa el hardware de red: `WIFI: Conectando a Livebox6...` y obtiene una dirección IP local dentro del rango de la red.
2. Se inicializa el sensor de imagen con éxito: `cam_hal: cam init ok` y el controlador asigna los buffers de captura correspondientes (`Allocating Byte frame buffer in PSRAM`).
3. El microcontrolador captura el frame, empaqueta los bytes en bruto y lanza una petición `POST /detect`.
4. El servidor YOLOv8 recibe los bytes de la imagen (`DEBUG: Bytes recibidos = 2255`), ejecuta la inferencia en milisegundos (`Speed: 15.5ms preprocess, 120.2ms inference`) y devuelve un estado `200 OK` con el recuento de personas ubicadas en sus respectivas *bounding boxes*.

A continuación, se detalla en la consola del dispositivo el log completo que confirma la correcta asignación de memoria y estabilización de la red inalámbrica:

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/f5998bce37bcd23365319a2e8ed8f418928560ac/TFM_Imagenes/Captura%20de%20pantalla%202026-06-04%20152530.png)

En la siguiente captura se muestra el monitoreo activo del servidor FastAPI procesando los frames entrantes del ESP32-S3 Eye y calculando las detecciones:

![imagen alt](https://github.com/Gisspat/TFM_2026_Master_IoT/blob/f5998bce37bcd23365319a2e8ed8f418928560ac/TFM_Imagenes/Captura%20de%20pantalla%202026-06-04%20152503.png)

## 8. Conclusiones Finales

* **Rendimiento Óptimo:** La integración de la detección utilizando un servidor dedicado en combinación con FastAPI permite procesar imágenes con tiempos de inferencia sumamente bajos (~120ms), ideal para un monitoreo dinámico en transporte público.
* **Eficiencia de Hardware:** Delegar la carga pesada de procesamiento matemático (YOLO) al servidor y mantener al ESP32-S3 únicamente capturando y transmitiendo en bruto demuestra ser una arquitectura eficiente para soluciones IoT de bajo costo y consumo equilibrado.
* **Escalabilidad:** El sistema cuenta con la capacidad de trabajar de forma continua e inalámbrica, permitiendo reubicar la cámara en cualquier sección del autobús que cuente con cobertura de red.
* **Sincronización Concurrente Excepcional:** La arquitectura asíncrona de FastAPI previene bloqueos en el procesamiento del flujo de frames. Esto asegura que el servidor pueda atender múltiples peticiones concurrentes desde el nodo IoT sin retrasar el tiempo real del transporte.
* **Viabilidad de Despliegue Real:** La combinación de hardware de bajo costo (ESP32-S3) con un modelo preentrenado optimizado de última generación demuestra que es perfectamente viable digitalizar el aforo de vehículos de transporte público con un presupuesto reducido, sin sacrificar robustez ni precisión en la detección.
