#      CONFIGURACIÓN GLOBAL Y DIRECTOR DE COMPILACIÓN DEL PROYECTO
# 1. Especificación de la versión mínima requerida de la herramienta CMake
cmake_minimum_required(VERSION 3.16)
# 2. Inyección de las macros, scripts y herramientas del Core de ESP-IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# 3. Definición y registro del nombre del firmware global del autobús
project(INTERNET)
