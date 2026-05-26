# Introducción

## Especificación del proyecto

BrakeBike es un prototipo TinyML/IoT para la detección de eventos de conducción en bicicleta mediante una unidad de medida inercial, IMU, montada en la zona del manillar.

El sistema utiliza una **Arduino Nano 33 BLE Sense**, que incorpora una IMU LSM9DS1 con acelerómetro y giróscopo. A partir de estas señales se entrena un modelo en **Edge Impulse** para clasificar eventos de conducción y activar salidas visuales en la propia placa.

## Objetivo

El objetivo inicial del proyecto era detectar varios eventos ciclistas mediante señales inerciales:

- frenada suave,
- frenada fuerte,
- subida de bordillo,
- bajada de bordillo.

Durante el desarrollo práctico se simplificó la taxonomía para obtener un sistema más robusto y demostrable. La versión final implementada detecta:

- `normal`
- `frenada_fuerte`
- `subir_bordillo`

La clase `frenada_suave` se descartó porque, durante el análisis de datos, se observó que se confundía de forma significativa con la clase `normal`. La subida de bordillo se implementó como una simulación mediante levantamiento de la rueda delantera, al estar el sensor situado en la zona del manillar.

## Relación con el estado de la cuestión

El proyecto está relacionado con el manuscrito de revisión sistemática realizado sobre detección de eventos ciclistas mediante IMU.

En dicho estado de la cuestión se revisan métodos basados en acelerómetros, giróscopos e IMU para detectar eventos como frenadas, caídas, near-miss, anomalías del firme y maniobras de conducción. Las conclusiones del manuscrito apoyan el enfoque seguido en este proyecto: utilizar una IMU fija en la bicicleta y un clasificador ligero basado en ventanas temporales de aceleración y giro.

## Manuscrito del estado de la cuestión

[Ver manuscrito del estado de la cuestión](./manuscrito_estado_cuestion.pdf)

## Resumen del sistema implementado

El flujo general del sistema es:

1. La Arduino Nano 33 BLE Sense se fija en el manillar.
2. Se capturan señales de acelerómetro y giróscopo.
3. Los datos se envían por puerto serie al ordenador.
4. Python guarda las muestras en archivos CSV.
5. Los datos se suben a Edge Impulse.
6. Se entrena un modelo con ventanas temporales de 1500 ms.
7. El modelo se exporta como librería Arduino cuantizada en int8.
8. La placa ejecuta inferencia en tiempo real y activa LEDs según la clase detectada.

## Diagrama general

![Diagrama del sistema](../docs/fig_diagrama_sistema.png)