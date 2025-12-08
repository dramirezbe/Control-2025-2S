# Control-2025-2S 🚀

Bienvenido al repositorio oficial del curso de **Sistemas de Control (Semestre 2025-2S)**. Este espacio centraliza el desarrollo de talleres, laboratorios y el proyecto final de la asignatura, enfocándose en el diseño, análisis y simulación de controladores.

## 📋 Tabla de Contenidos

- [Control-2025-2S 🚀](#control-2025-2s-)
  - [📋 Tabla de Contenidos](#-tabla-de-contenidos)
  - [📖 Descripción del Proyecto](#-descripción-del-proyecto)
  - [📂 Estructura del Repositorio](#-estructura-del-repositorio)
    - [🎓 **ProyectoFinal/**](#-proyectofinal)
    - [🛠️ **taller1/**](#️-taller1)
    - [🛠️ **taller3/**](#️-taller3)
  - [💻 Tecnologías y Herramientas](#-tecnologías-y-herramientas)
  - [⚙️ Requisitos e Instalación](#️-requisitos-e-instalación)
    - [Prerrequisitos](#prerrequisitos)
    - [Instalación de Librerías](#instalación-de-librerías)

---

## 📖 Descripción del Proyecto

Este repositorio aloja el código fuente y la documentación técnica generada durante el semestre. El objetivo principal es aplicar la teoría de control clásico y moderno a sistemas dinámicos, utilizando herramientas computacionales para la simulación y validación de resultados.

El enfoque incluye:
* Modelado matemático de sistemas físicos.
* Análisis de estabilidad y respuesta en el tiempo/frecuencia.
* Diseño e implementación de controladores (PID, LGR, Bode).

---

## 📂 Estructura del Repositorio

El contenido se organiza en módulos de aprendizaje progresivo:

### 🎓 **ProyectoFinal/**
Es el núcleo práctico del curso. Integra todos los conocimientos adquiridos para resolver un problema de ingeniería complejo.
* **Contenido esperado:**
    * 📘 **Notebooks de Análisis:** Archivos `.ipynb` con el modelado matemático del sistema, diseño del controlador y gráficas de simulación comparativa.
    * ⚙️ **Implementación (C/C++):** Dado que el repositorio contiene código en **C**, es probable que esta carpeta incluya scripts para sistemas embebidos (como Arduino o ESP32) que ejecutan la ley de control en tiempo real.
    * 📄 **Reportes:** Documentación técnica y conclusiones.

### 🛠️ **taller1/**
Primer módulo práctico. Se enfoca en los fundamentos.
* **Temáticas probables:** Introducción a Python para control, álgebra de bloques, y análisis de respuesta transitoria (sistemas de primer y segundo orden).

### 🛠️ **taller3/**
Módulo avanzado de diseño.
* **Temáticas probables:** Técnicas de diseño de compensadores, Lugar Geométrico de las Raíces (LGR) o análisis de respuesta en frecuencia (Diagramas de Bode/Nyquist).

---

## 💻 Tecnologías y Herramientas

El proyecto hace uso de un stack híbrido para simulación e implementación:

* ![Jupyter](https://img.shields.io/badge/Jupyter-Notebook-F37626?style=flat&logo=jupyter&logoColor=white) **Jupyter Notebook (96.8%)**: Herramienta principal para la documentación ejecutable, cálculos simbólicos y visualización de datos.
* ![C](https://img.shields.io/badge/C-00599C?style=flat&logo=c&logoColor=white) **Lenguaje C (1.6%)**: Utilizado probablemente para la implementación discreta de controladores o programación de microcontroladores.
* ![Python](https://img.shields.io/badge/Python-3776AB?style=flat&logo=python&logoColor=white) **Python (1.4%)**: Scripts auxiliares y librerías de cálculo.

---

## ⚙️ Requisitos e Instalación

Para ejecutar los notebooks y scripts de este repositorio, necesitarás configurar un entorno de Python.

### Prerrequisitos
* **Python 3.8+**
* **Git**

### Instalación de Librerías
Se recomienda crear un entorno virtual e instalar las dependencias clave para el análisis de sistemas de control:

```bash
# Clonar el repositorio
git clone [https://github.com/dramirezbe/Control-2025-2S.git](https://github.com/dramirezbe/Control-2025-2S.git)
cd Control-2025-2S

# Instalación de dependencias sugeridas
pip install numpy matplotlib scipy control jupyterlab
```
