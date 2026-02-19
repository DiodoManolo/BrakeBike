# BrakeBike

Proyecto IoT basado en un sensor inercial (IMU: acelerómetro + giroscopio) para detectar eventos de conducción en bicicleta.

## Objetivo
Detectar en tiempo real estos **4 eventos** a partir de señales inerciales:

1. **Frenada suave**
2. **Frenada fuerte**
3. **Subir bordillo**
4. **Bajar bordillo**


## Figura (generada con IA)
![Diagrama del sistema](docs/fig_diagrama_sistema.png)

> Figura generada con IA: diagrama de bloques del flujo del sistema.


## Montaje del sensor
Ubicación prevista del IMU: **(ejemplo: potencia/manillar o tubo superior)**  
Fijación: soporte rígido + bridas / cinta doble cara para minimizar vibraciones parásitas.

## Dataset (base de datos)
La base de datos se generará manualmente realizando sesiones controladas y etiquetadas:

- Rodaje normal (sin eventos) para “background”.
- Frenadas suaves (repetidas).
- Frenadas fuertes (repetidas).
- Bordillo: subir y bajar (repeticiones).



## Estado del proyecto
- [ ] Repo creado y estructura inicial
- [ ] Figura IA en `docs/`
- [ ] Extracción de datos IMU
- [ ] Dataset v1 (grabación + etiquetas)
- [ ] Análisis v1 (features + gráficas)
- [ ] Implementación final (detección en la placa)