#include <WiFi.h>
#include <HTTPClient.h>
#include <HX711.h>
#include <LiquidCrystal_I2C.h>

// ---------------- Wi-Fi & Telegram ----------------
const char* ssid = "AirFiber-Rov1ne";
const char* password = "mahBeegahY9quo9O";

String botToken = "8433647240:AAHG3dhekOhAF-9EZCwHs83hsKsq9nwDFjo";
String chatID []= {"2073294608", "1613498408"};

// ---------------- Pins ----------------
#define DOUT 4
#define SCK 5
#define LED_PIN 25
#define BUZZER_PIN 26
#define BTN_HUNDREDS 32
#define BTN_TENS 33
#define BTN_UNITS 27

// ---------------- Peripherals ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;
float calibration_factor = 693950.0;
float bottleWeight = 0.0;

// ---------------- Threshold & state ----------------
int threshold_hundreds = 0;
int threshold_tens = 0;
int threshold_units = 0;
float threshold_ml = 0.0;
bool thresholdSet = false;
bool monitoringStarted = false;

unsigned long inputTimeout = 20000;       // ms
const unsigned long debounceDelay = 200;  // ms

// alerts state
bool last25_alert_sent[3] = { false, false, false };
bool finalAlertSent = false;

String patientName = "John Doe";

// prototypes
float readStableWeight();
void sendTelegramMessage(String message);
void triggerAlarm();
void triggerAlarm_Final();
void setup25PercentNotifications();
void check25PercentNotifications(float current_ml, float threshold_ml);

// ---------------- setup ----------------
void setup() {
  Serial.begin(115200);

  // pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_HUNDREDS, INPUT_PULLUP);
  pinMode(BTN_TENS, INPUT_PULLUP);
  pinMode(BTN_UNITS, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);

  // LCD
  lcd.init();
  lcd.backlight();

  // WiFi
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Connected");
  delay(700);

  // HX711
  scale.begin(DOUT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();

  // ---------- WAIT FOR BOTTLE ----------
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place IV Bottle");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...      ");

  float stableWeight = readStableWeight();
  bool bottleDetected = false;
  bool lcdUpdated = false;

  while (!bottleDetected) {
    stableWeight = readStableWeight();

    if (stableWeight < 50.0) {
      if (!lcdUpdated) {
        lcd.setCursor(0, 0);
        lcd.print("⚠ Bottle too light");
        lcd.setCursor(0, 1);
        lcd.print("Waiting...      ");
        Serial.println("⚠ Bottle too light! Waiting...");
        lcdUpdated = true;
      }

      // Continuous buzzer beep
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      delay(200);

    } else {
      bottleDetected = true;
      lcdUpdated = false;
    }
  }

  bottleWeight = stableWeight;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bottle OK:      ");
  lcd.setCursor(0, 1);
  lcd.print(bottleWeight, 0);
  lcd.print(" ml     ");
  Serial.printf("✅ Bottle detected: %.2f ml\n", bottleWeight);

  // confirmation beep
  digitalWrite(BUZZER_PIN, HIGH);
  delay(800);
  digitalWrite(BUZZER_PIN, LOW);

  // ---------- Threshold input (live) ----------
  lcd.clear();
  lcd.print("Set threshold:");
  lcd.setCursor(0, 1);
  lcd.print("Thr: 000 ml");

  unsigned long startTime = millis();
  unsigned long lastHundreds = 0, lastTens = 0, lastUnits = 0;
  threshold_hundreds = 0;
  threshold_tens = 0;
  threshold_units = 0;
  thresholdSet = false;
  static int lastDisplayedThreshold = -1;

  while (millis() - startTime < inputTimeout) {
    unsigned long now = millis();
    bool updated = false;

    if (digitalRead(BTN_HUNDREDS) == LOW && (now - lastHundreds > debounceDelay)) {
      threshold_hundreds = (threshold_hundreds + 1) % 10;
      lastHundreds = now;
      updated = true;
    }
    if (digitalRead(BTN_TENS) == LOW && (now - lastTens > debounceDelay)) {
      threshold_tens = (threshold_tens + 1) % 10;
      lastTens = now;
      updated = true;
    }
    if (digitalRead(BTN_UNITS) == LOW && (now - lastUnits > debounceDelay)) {
      threshold_units = (threshold_units + 1) % 10;
      lastUnits = now;
      updated = true;
    }

    if (updated) {
      threshold_ml = threshold_hundreds * 100 + threshold_tens * 10 + threshold_units;
      thresholdSet = true;

      if ((int)threshold_ml != lastDisplayedThreshold) {
        lcd.setCursor(0, 1);
        lcd.print("Thr: ");
        if (threshold_ml < 10) lcd.print("00");
        else if (threshold_ml < 100) lcd.print("0");
        lcd.print((int)threshold_ml);
        lcd.print(" ml   ");
        lastDisplayedThreshold = (int)threshold_ml;

        Serial.printf("Threshold live: %03d ( %0.0f ml )\n", (int)threshold_ml, threshold_ml);
      }
    }

    delay(50);
  }

  if (!thresholdSet) {
    threshold_ml = 70.0;
    lcd.clear();
    lcd.print("Auto Thr: 70ml");
    Serial.println("No input — using default 70 ml");
    delay(1200);
  } else {
    Serial.printf("Final threshold: %.0f ml\n", threshold_ml);
    lcd.clear();
    lcd.print("Set Thr:");
    lcd.setCursor(0, 1);
    lcd.print((int)threshold_ml);
    lcd.print(" ml");
    delay(900);
  }

  // send initial telegram
  String initMsg = "IV Monitoring Started\nPatient: " + patientName + "\nBottle Weight: " + String(bottleWeight, 1) + " g\nThreshold: " + String(threshold_ml, 1) + " ml";
  sendTelegramMessage(initMsg);

  setup25PercentNotifications();
  lcd.clear();
  lcd.print("Monitoring...");
  delay(800);
  monitoringStarted = true;
}

// ---------------- loop ----------------
void loop() {
  if (!monitoringStarted) return;

  float weight_g = readStableWeight();

  if (weight_g < 5.0) {
    lcd.clear();
    lcd.print("No Bottle Found");
    Serial.println("⚠ No bottle detected...");
    delay(1000);
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("Now: ");
  lcd.print(weight_g, 1);
  lcd.print("g   ");
  lcd.setCursor(0, 1);
  lcd.print("Thr: ");
  lcd.print(threshold_ml, 0);
  lcd.print("ml   ");

  Serial.print("Weight: ");
  Serial.print(weight_g, 1);
  Serial.print(" g | Threshold: ");
  Serial.println(threshold_ml, 1);

  check25PercentNotifications(weight_g, threshold_ml);
  delay(800);
}

// ---------------- helpers ----------------
float readStableWeight() {
  float w = scale.get_units(10) * 1000.0;
  if (abs(w) < 5.0) w = 0.0;
  return w;
}

void setup25PercentNotifications() {
  for (int i = 0; i < 3; i++) last25_alert_sent[i] = false;
  finalAlertSent = false;
}

void check25PercentNotifications(float current_ml, float threshold_ml) {
  float used_ml = bottleWeight - current_ml;
  float alert1 = threshold_ml * 0.25;
  float alert2 = threshold_ml * 0.5;
  float alert3 = threshold_ml * 0.75;

  if (!last25_alert_sent[0] && used_ml >= alert1) {
    last25_alert_sent[0] = true;
    sendTelegramMessage("⚠ IV Alert 1: " + patientName + "\nRemaining: " + String(current_ml, 1) + " ml");
    triggerAlarm();
  }
  if (!last25_alert_sent[1] && used_ml >= alert2) {
    last25_alert_sent[1] = true;
    sendTelegramMessage("⚠ IV Alert 2: " + patientName + "\nRemaining: " + String(current_ml, 1) + " ml");
    triggerAlarm();
  }
  if (!last25_alert_sent[2] && used_ml >= alert3 && !finalAlertSent) {
    last25_alert_sent[2] = true;
    finalAlertSent = true;
    sendTelegramMessage("⚠ IV Final Alert: " + patientName + "\nRemaining: " + String(current_ml, 1) + " ml");
    triggerAlarm_Final();
    sendTelegramMessage("✅ IV Threshold completed for " + patientName + " at " + String(current_ml,1) + " ml");
  }
}

void triggerAlarm() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

void triggerAlarm_Final() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);
  }
}

void sendTelegramMessage(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    message.replace("\n", "%0A");
    message.replace(" ", "%20");
    String url = "https://api.telegram.org/bot" + botToken +
             "/sendMessage?chat_id=" + chatIDs[i] + "&text=" + message;

    http.begin(url);
    int code = http.GET();
    if (code > 0) Serial.println("✅ Telegram message sent!");
    else Serial.println("❌ Telegram send failed");
    http.end();
  }
}
