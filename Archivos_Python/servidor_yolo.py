from fastapi import FastAPI, Request
from ultralytics import YOLO
import cv2
import numpy as np
import os

app = FastAPI()
# Carga del  modelo entrenado
model = YOLO("best.pt")

# Creamos una carpeta para ver los resultados
OUTPUT_DIR = "capturas_esp32"
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

@app.post("/detect")
async def detect(request: Request):
    # Recepción de bytes enviados por el ESP32-S3-EYE
    img_bytes = await request.body()
    
    print(f"DEBUG: Bytes recibidos = {len(img_bytes)}")

    if len(img_bytes) < 100:
        return {"person_count": 0, "status": "empty_buffer"}

    # Convertir a array de numpy
    nparr = np.frombuffer(img_bytes, np.uint8)
    # Decodificar JPEG -> Matriz OpenCV
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    if img is None:
        # Si llega aquí, los bytes no son un JPEG válido
        print("ERROR: No se pudo decodificar la imagen. ¿Es formato JPEG?")
        return {"person_count": 0, "status": "corrupt_jpeg"}

    # Inferencia con YOLOv8
    results = model(img)
    count = len(results[0].boxes)
    
    # Guardar para verificar
    annotated = results[0].plot()
    cv2.imwrite("capturas_esp32/ultimo_frame.jpg", annotated)
    
    #Respuesta JSON

    return {"person_count": count}