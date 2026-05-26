# Análisis de datos

## Objetivo

En esta sección se documenta el análisis realizado sobre los datos recogidos, las decisiones tomadas en Edge Impulse y la evolución del modelo hasta llegar a la versión final.

## Datos disponibles

Los datos se organizaron en carpetas por clase:

```text
analisis/datos_crudos/
├── normal/
├── frenada_fuerte/
└── subir_bordillo/
```

Durante la fase inicial también se recogieron muestras de `frenada_suave`, pero se descartaron del modelo final por baja separabilidad frente a `normal`.

## Procesamiento en Edge Impulse

La configuración principal del impulso fue:

- Tipo de datos: series temporales.
- Frecuencia: 100 Hz.
- Tamaño de ventana: 1500 ms.
- Señales de entrada:
  - `accX`
  - `accY`
  - `accZ`
  - `gyrX`
  - `gyrY`
  - `gyrZ`
- Bloque de procesado: **Spectral Analysis**.
- Bloque de aprendizaje: **Classification**.
- Deployment final: **Arduino Library**, modelo cuantizado `int8`.

## Modelo inicial con frenada suave

En el primer modelo se incluyó la clase `frenada_suave`. El resultado mostró que esta clase se confundía con `normal`.

Interpretación:

- `frenada_fuerte` presentaba una firma inercial clara.
- `subir_bordillo` presentaba una firma clara por el levantamiento de la rueda delantera.
- `frenada_suave` tenía variaciones muy pequeñas y parecidas a la circulación normal.

Por este motivo, se decidió retirar `frenada_suave` para mejorar la robustez del prototipo final.

## Modelo final

El modelo final utiliza tres clases:

- `normal`
- `frenada_fuerte`
- `subir_bordillo`

Con esta configuración se obtuvo una separación mucho mejor entre clases, especialmente entre eventos bruscos y circulación normal.

## Gráficas recomendadas para incluir

Colocar en `analisis/graficas/` las capturas exportadas de Edge Impulse:

```text
analisis/graficas/
├── confusion_matrix_con_frenada_suave.png
├── confusion_matrix_final_training.png
├── confusion_matrix_final_testing.png
├── feature_explorer_con_frenada_suave.png
└── feature_explorer_final.png
```

## Conclusiones del análisis

1. El uso de acelerómetro y giróscopo permite detectar eventos bruscos de forma robusta.
2. La clase `frenada_fuerte` es distinguible por la deceleración y el cambio dinámico en la bicicleta.
3. La clase `subir_bordillo` es claramente separable al simular el evento levantando la rueda delantera.
4. La clase `frenada_suave` no se mantuvo porque se confundía con `normal`.
5. La reducción de clases mejoró el funcionamiento práctico de la demo.
6. El modelo final es suficientemente ligero para ejecutarse en la Arduino Nano 33 BLE Sense.

## Decisión final

La decisión más importante del análisis fue priorizar una clasificación más robusta frente a una taxonomía más ambiciosa. Por ello se redujo el problema a eventos más claramente detectables por la IMU.
