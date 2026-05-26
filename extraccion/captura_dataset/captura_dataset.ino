#include <Arduino_LSM9DS1.h>

// =======================
// PINES
// =======================
#define BTN_CLASS_PIN 2
#define BTN_RECORD_PIN 3
#define REC_LED_PIN 4

// LED RGB interno de Arduino Nano 33 BLE Sense
#define RED_PIN 22
#define GREEN_PIN 23
#define BLUE_PIN 24

// =======================
// CONFIGURACIÓN
// =======================
const unsigned long SAMPLE_PERIOD_US = 10000;      // 100 Hz = una muestra cada 10 ms
const unsigned long RECORD_DURATION_MS = 60000;    // 60 s. Para probar en casa puedes poner 10000

// =======================
// CLASES
// =======================
enum ClassId {
  NORMAL = 0,
  FRENADA_SUAVE,
  FRENADA_FUERTE,
  SUBIR_BORDILLO,
  NUM_CLASSES
};

const char* classNames[NUM_CLASSES] = {
  "normal",
  "frenada_suave",
  "frenada_fuerte",
  "subir_bordillo"
};

ClassId currentClass = NORMAL;

// =======================
// ESTADO DE GRABACIÓN
// =======================
bool recording = false;
unsigned long recordStartMs = 0;
unsigned long lastSampleUs = 0;
unsigned int recordCounter = 0;

// Variables IMU
float ax = 0, ay = 0, az = 0;
float gx = 0, gy = 0, gz = 0;

// =======================
// ANTIRREBOTE BOTONES
// =======================
struct DebouncedButton {
  int pin;
  int stableState;
  int lastReading;
  unsigned long lastChangeMs;
};

DebouncedButton btnClass = {BTN_CLASS_PIN, HIGH, HIGH, 0};
DebouncedButton btnRecord = {BTN_RECORD_PIN, HIGH, HIGH, 0};

const unsigned long DEBOUNCE_MS = 40;

bool buttonFell(DebouncedButton &btn) {
  int reading = digitalRead(btn.pin);

  if (reading != btn.lastReading) {
    btn.lastChangeMs = millis();
    btn.lastReading = reading;
  }

  if ((millis() - btn.lastChangeMs) > DEBOUNCE_MS) {
    if (reading != btn.stableState) {
      int previous = btn.stableState;
      btn.stableState = reading;

      // Con INPUT_PULLUP:
      // sin pulsar = HIGH
      // pulsado = LOW
      if (previous == HIGH && btn.stableState == LOW) {
        return true;
      }
    }
  }

  return false;
}

// =======================
// LED RGB INTERNO
// En Nano 33 BLE Sense normalmente es activo a nivel bajo:
// LOW enciende, HIGH apaga.
// =======================
void setRGB(bool redOn, bool greenOn, bool blueOn) {
  digitalWrite(RED_PIN, redOn ? LOW : HIGH);
  digitalWrite(GREEN_PIN, greenOn ? LOW : HIGH);
  digitalWrite(BLUE_PIN, blueOn ? LOW : HIGH);
}

void showCurrentClassColor() {
  switch (currentClass) {
    case NORMAL:
      setRGB(true, true, true);      // blanco
      break;

    case FRENADA_SUAVE:
      setRGB(false, false, true);    // azul
      break;

    case FRENADA_FUERTE:
      setRGB(true, false, false);    // rojo
      break;

    case SUBIR_BORDILLO:
      setRGB(false, true, false);    // verde
      break;

    default:
      setRGB(false, false, false);   // apagado
      break;
  }
}

// =======================
// CAMBIAR CLASE
// =======================
void nextClass() {
  currentClass = (ClassId)((currentClass + 1) % NUM_CLASSES);

  showCurrentClassColor();

  Serial.print("#CLASS,");
  Serial.println(classNames[currentClass]);
}

// =======================
// INICIAR GRABACIÓN
// =======================
void startRecording() {
  if (recording) return;

  recording = true;
  recordStartMs = millis();
  lastSampleUs = micros();
  recordCounter++;

  digitalWrite(REC_LED_PIN, HIGH);

  Serial.print("#START,");
  Serial.print(classNames[currentClass]);
  Serial.print(",");
  Serial.println(recordCounter);
}

// =======================
// PARAR GRABACIÓN
// =======================
void stopRecording() {
  if (!recording) return;

  recording = false;
  digitalWrite(REC_LED_PIN, LOW);

  Serial.print("#END,");
  Serial.print(classNames[currentClass]);
  Serial.print(",");
  Serial.println(recordCounter);
}

// =======================
// LEER IMU
// =======================
void updateIMU() {
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
  }

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
  }
}

// =======================
// SETUP
// =======================
void setup() {
  pinMode(BTN_CLASS_PIN, INPUT_PULLUP);
  pinMode(BTN_RECORD_PIN, INPUT_PULLUP);
  pinMode(REC_LED_PIN, OUTPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  digitalWrite(REC_LED_PIN, LOW);
  setRGB(false, false, false);

  Serial.begin(230400);

  // Espera corta al puerto serie
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0 < 5000)) {
    delay(10);
  }

  if (!IMU.begin()) {
    Serial.println("#ERROR,IMU_no_inicializada");
    setRGB(true, false, false); // rojo
    while (1) {
      delay(100);
    }
  }

  showCurrentClassColor();

  Serial.println("#READY");
  Serial.println("#Formato: DATA,timestamp_ms,accX,accY,accZ,gyrX,gyrY,gyrZ");
  Serial.print("#Clase inicial,");
  Serial.println(classNames[currentClass]);
}

// =======================
// LOOP PRINCIPAL
// =======================
void loop() {
  updateIMU();

  // Cambiar clase solo si no estamos grabando
  if (buttonFell(btnClass) && !recording) {
    nextClass();
  }

  // Empezar/parar grabación
  if (buttonFell(btnRecord)) {
    if (!recording) {
      startRecording();
    } else {
      stopRecording();
    }
  }

  // Si estamos grabando, enviar muestras a 100 Hz
  if (recording) {
    unsigned long nowMs = millis();

    // Parada automática al llegar a la duración
    if (nowMs - recordStartMs >= RECORD_DURATION_MS) {
      stopRecording();
      return;
    }

    unsigned long nowUs = micros();

    if (nowUs - lastSampleUs >= SAMPLE_PERIOD_US) {
      lastSampleUs += SAMPLE_PERIOD_US;

      unsigned long tRel = nowMs - recordStartMs;

      Serial.print("DATA,");
      Serial.print(tRel);
      Serial.print(",");
      Serial.print(ax, 6);
      Serial.print(",");
      Serial.print(ay, 6);
      Serial.print(",");
      Serial.print(az, 6);
      Serial.print(",");
      Serial.print(gx, 6);
      Serial.print(",");
      Serial.print(gy, 6);
      Serial.print(",");
      Serial.println(gz, 6);
    }
  }
}