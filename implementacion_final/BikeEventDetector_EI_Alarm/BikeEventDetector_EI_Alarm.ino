#include <ManuelGomez-project-1_inferencing.h>
#include <Arduino_LSM9DS1.h>

// =======================================================
// PINES
// =======================================================

// LED RGB interno Arduino Nano 33 BLE Sense
#define RED_PIN     22
#define GREEN_PIN   23
#define BLUE_PIN    24

// LEDs externos
#define EXT_RED_LED_PIN      4   // LED rojo externo: alarma frenada fuerte
#define CAPTURE_GREEN_LED    6   // LED verde externo: capturando ventana

// =======================================================
// UMBRALES MODIFICABLES
// =======================================================

// Si detecta frenada fuerte todo el rato, sube este valor a 0.95
// Si no detecta frenadas reales, bájalo a 0.80
#define TH_FRENADA_FUERTE  0.90f

// Si no detecta subir bordillo, bájalo a 0.55
// Si lo detecta sin querer, súbelo a 0.75
#define TH_SUBIR_BORDILLO  0.65f

// Umbral para considerar normal.
// Si no hay ninguna clase clara, el código también pondrá estado normal.
#define TH_NORMAL          0.50f

// Margen extra para que frenada_fuerte tenga que ganar claramente.
// Si sigue detectando frenada fuerte todo el rato, sube a 0.15 o 0.20
#define TH_MARGIN_FUERTE   0.10f

// Buffer de entrada para Edge Impulse
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

// =======================================================
// LED RGB INTERNO
// En Nano 33 BLE Sense normalmente:
// LOW = encendido
// HIGH = apagado
// =======================================================

void setRGB(bool redOn, bool greenOn, bool blueOn) {
  digitalWrite(RED_PIN, redOn ? LOW : HIGH);
  digitalWrite(GREEN_PIN, greenOn ? LOW : HIGH);
  digitalWrite(BLUE_PIN, blueOn ? LOW : HIGH);
}

// =======================================================
// LEDS EXTERNOS
// =======================================================

void setRedAlarmLed(bool on) {
  digitalWrite(EXT_RED_LED_PIN, on ? HIGH : LOW);
}

void setCaptureLed(bool on) {
  digitalWrite(CAPTURE_GREEN_LED, on ? HIGH : LOW);
}

// =======================================================
// ESTADOS VISUALES
// =======================================================

void setStateNormal() {
  // Normal o desconocido:
  // RGB blanco, LED rojo externo apagado
  setRGB(true, true, true);
  setRedAlarmLed(false);
}

void setStateFrenadaFuerte() {
  // Frenada fuerte:
  // RGB rojo + LED rojo externo encendido
  setRGB(true, false, false);
  setRedAlarmLed(true);
  Serial.println("ALERTA,FRENADA_FUERTE");
}

void setStateSubirBordillo() {
  // Subir bordillo:
  // RGB verde, LED rojo externo apagado
  setRGB(false, true, false);
  setRedAlarmLed(false);
}

// =======================================================
// DECISIÓN POR UMBRALES SEPARADOS
// =======================================================

void decideByThresholds(ei_impulse_result_t &result) {
  float pNormal = 0.0f;
  float pFrenadaFuerte = 0.0f;
  float pSubirBordillo = 0.0f;

  // Buscar probabilidades por nombre de etiqueta
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    String label = String(result.classification[i].label);
    label.toLowerCase();

    float value = result.classification[i].value;

    if (label.indexOf("normal") >= 0) {
      pNormal = value;
    }
    else if (label.indexOf("frenada_fuerte") >= 0) {
      pFrenadaFuerte = value;
    }
    else if (label.indexOf("subir_bordillo") >= 0) {
      pSubirBordillo = value;
    }
  }

  Serial.print("pNormal=");
  Serial.print(pNormal, 3);
  Serial.print(" | pFrenadaFuerte=");
  Serial.print(pFrenadaFuerte, 3);
  Serial.print(" | pSubirBordillo=");
  Serial.println(pSubirBordillo, 3);

  // 1) Frenada fuerte: umbral alto + debe ganar con margen
  if (pFrenadaFuerte >= TH_FRENADA_FUERTE &&
      pFrenadaFuerte >= (pNormal + TH_MARGIN_FUERTE) &&
      pFrenadaFuerte >= (pSubirBordillo + TH_MARGIN_FUERTE)) {

    Serial.println("DECISION: frenada_fuerte");
    setStateFrenadaFuerte();
  }

  // 2) Subir bordillo
  else if (pSubirBordillo >= TH_SUBIR_BORDILLO &&
           pSubirBordillo >= pNormal &&
           pSubirBordillo >= pFrenadaFuerte) {

    Serial.println("DECISION: subir_bordillo");
    setStateSubirBordillo();
  }

  // 3) Normal explícito
  else if (pNormal >= TH_NORMAL) {
    Serial.println("DECISION: normal");
    setStateNormal();
  }

  // 4) Si no hay nada claro, lo tratamos como normal/desconocido
  else {
    Serial.println("DECISION: desconocido -> normal");
    setStateNormal();
  }
}

// =======================================================
// SETUP
// =======================================================

void setup() {
  Serial.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  pinMode(EXT_RED_LED_PIN, OUTPUT);
  pinMode(CAPTURE_GREEN_LED, OUTPUT);

  setRGB(false, false, false);
  setRedAlarmLed(false);
  setCaptureLed(false);

  if (!IMU.begin()) {
    Serial.println("ERROR: No se pudo inicializar la IMU");
    setRGB(true, false, false); // rojo fijo = error
    while (1) {
      delay(100);
    }
  }

  Serial.println("Sistema Edge Impulse listo");
  Serial.println("-----------------------------------");

  Serial.print("EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE: ");
  Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);

  Serial.print("EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME: ");
  Serial.println(EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME);

  Serial.print("EI_CLASSIFIER_INTERVAL_MS: ");
  Serial.println(EI_CLASSIFIER_INTERVAL_MS);

  Serial.print("Numero de clases: ");
  Serial.println(EI_CLASSIFIER_LABEL_COUNT);

  Serial.println("-----------------------------------");

  Serial.println("Umbrales:");
  Serial.print("TH_FRENADA_FUERTE = ");
  Serial.println(TH_FRENADA_FUERTE, 2);
  Serial.print("TH_SUBIR_BORDILLO = ");
  Serial.println(TH_SUBIR_BORDILLO, 2);
  Serial.print("TH_NORMAL = ");
  Serial.println(TH_NORMAL, 2);
  Serial.print("TH_MARGIN_FUERTE = ");
  Serial.println(TH_MARGIN_FUERTE, 2);

  Serial.println("-----------------------------------");

  if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 6) {
    Serial.println("AVISO: El modelo no parece tener 6 ejes.");
    Serial.println("Debe coincidir con: accX, accY, accZ, gyrX, gyrY, gyrZ");
  }

  // Estado inicial
  setStateNormal();
}

// =======================================================
// LOOP PRINCIPAL
// =======================================================

void loop() {
  Serial.println("Capturando ventana...");

  // LED verde externo encendido mientras captura la ventana
  setCaptureLed(true);

  // Captura de una ventana completa.
  // Si el modelo es de 1500 ms a 100 Hz y 6 ejes:
  // 150 muestras * 6 señales = 900 valores.
  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {

    uint64_t next_tick = micros() + ((uint64_t)EI_CLASSIFIER_INTERVAL_MS * 1000);

    float ax, ay, az;
    float gx, gy, gz;

    // Espera a que haya datos nuevos de acelerómetro y giroscopio
    while (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable()) {
      delay(1);
    }

    IMU.readAcceleration(ax, ay, az); // en g, igual que el dataset
    IMU.readGyroscope(gx, gy, gz);    // en grados/s, igual que el dataset

    // Orden igual que en el CSV:
    // timestamp,accX,accY,accZ,gyrX,gyrY,gyrZ
    features[ix + 0] = ax;
    features[ix + 1] = ay;
    features[ix + 2] = az;
    features[ix + 3] = gx;
    features[ix + 4] = gy;
    features[ix + 5] = gz;

    int64_t wait_time = next_tick - micros();
    if (wait_time > 0) {
      delayMicroseconds(wait_time);
    }
  }

  // LED verde externo apagado al terminar la captura
  setCaptureLed(false);

  Serial.println("Ventana capturada. Ejecutando clasificador...");

  signal_t signal;
  int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

  if (err != 0) {
    Serial.print("ERROR signal_from_buffer: ");
    Serial.println(err);
    setStateNormal();
    return;
  }

  ei_impulse_result_t result = { 0 };

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  if (res != EI_IMPULSE_OK) {
    Serial.print("ERROR run_classifier: ");
    Serial.println(res);
    setStateNormal();
    return;
  }

  Serial.println("Resultados:");

  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    Serial.print("  ");
    Serial.print(result.classification[i].label);
    Serial.print(": ");
    Serial.println(result.classification[i].value, 3);
  }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
  Serial.print("Anomaly score: ");
  Serial.println(result.anomaly, 3);
#endif

  decideByThresholds(result);

  Serial.println();
}