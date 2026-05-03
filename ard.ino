#include <DHT.h>

// ─────────────────────────────
// SENSOR PINS
// ─────────────────────────────
#define FLAME_SENSOR A0
#define MQ2_SENSOR   A1
#define DHTPIN       A2
#define DHTTYPE      DHT11

// ─────────────────────────────
// RELAY PINS
// ─────────────────────────────
#define PUMP_RELAY   8
#define BUZZER_RELAY 9
#define FAN_RELAY    10

// ─────────────────────────────
// THRESHOLDS
// ─────────────────────────────
#define SMOKE_WARNING 300
#define SMOKE_FIRE    600

#define TEMP_WARNING  35
#define TEMP_FIRE     40

#define FLAME_THRESHOLD 500

DHT dht(DHTPIN, DHTTYPE);

// ─────────────────────────────
// SYSTEM STATE
// ─────────────────────────────
enum SystemState {
  NORMAL,
  WARNING,
  FIRE
};

SystemState state = NORMAL;

// ─────────────────────────────
// SETUP
// ─────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(BUZZER_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);

  // OFF (ACTIVE LOW RELAY)
  digitalWrite(PUMP_RELAY, HIGH);
  digitalWrite(BUZZER_RELAY, HIGH);
  digitalWrite(FAN_RELAY, HIGH);

  dht.begin();

  Serial.println("🔥 Fire System Started");
}

// ─────────────────────────────
// LOOP
// ─────────────────────────────
void loop() {

  // =========================
  // READ SENSORS
  // =========================
  int flameValue = analogRead(FLAME_SENSOR);
  int smokeValue = analogRead(MQ2_SENSOR);
  float temp = dht.readTemperature();

  if (isnan(temp)) {
    Serial.println("DHT ERROR");
    delay(1000);
    return;
  }

  int flameDetected = (flameValue < FLAME_THRESHOLD) ? 1 : 0;

  // =========================
  // PRIORITY LOGIC (IMPORTANT)
  // =========================

  // 🔴 FIRE FIRST (HIGHEST PRIORITY)
  if (flameDetected == 1 ||
      (smokeValue >= SMOKE_FIRE && temp >= TEMP_FIRE)) {

    state = FIRE;
  }

  // 🟡 WARNING (ONLY IF NOT FIRE)
  else if (smokeValue >= SMOKE_WARNING || temp >= TEMP_WARNING) {

    state = WARNING;
  }

  // 🟢 NORMAL
  else {

    state = NORMAL;
  }

  // =========================
  // ACTUATORS
  // =========================
  switch (state) {

    case NORMAL:
      digitalWrite(PUMP_RELAY, HIGH);
      digitalWrite(BUZZER_RELAY, HIGH);
      digitalWrite(FAN_RELAY, HIGH);
      break;

    case WARNING:
      digitalWrite(PUMP_RELAY, HIGH);
      digitalWrite(BUZZER_RELAY, LOW);
      digitalWrite(FAN_RELAY, LOW);
      break;

    case FIRE:
      digitalWrite(PUMP_RELAY, LOW);
      digitalWrite(BUZZER_RELAY, LOW);
      digitalWrite(FAN_RELAY, LOW);
      break;
  }

  // =========================
  // SERIAL OUTPUT
  // =========================
  Serial.println("=================================");

  Serial.print("Flame: ");
  Serial.println(flameDetected);

  Serial.print("Smoke: ");
  Serial.println(smokeValue);

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("STATE: ");

  if (state == NORMAL) Serial.println("NORMAL");
  else if (state == WARNING) Serial.println("WARNING");
  else Serial.println("FIRE");

  Serial.println("=================================\n");

  delay(1000);
}
