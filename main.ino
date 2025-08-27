#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "config.h"   // <-- load secrets here

// ============ PINS ============
#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ135_A 36
#define MQ135_D 33
#define TRIG_PIN 13
#define ECHO_PIN 12
#define VIBRATION_PIN 32
#define LASER_PIN 26
#define BUZZER_PIN 25
#define LED_PIN 2

// ============ OBJECTS ============
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============ BLYNK ============
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;

bool wifiConnected = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  lcd.begin(16, 2);
  lcd.backlight();

  dht.begin();
  pinMode(MQ135_D, INPUT);
  pinMode(VIBRATION_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LASER_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, pass);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
    delay(500);
    lcd.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Blynk.begin(auth, ssid, pass);
    wifiConnected = true;
    lcd.clear();
    lcd.print("WiFi OK");
  } else {
    wifiConnected = false;
    lcd.clear();
    lcd.print("WiFi FAIL");
  }

  delay(1000);
}

void loop() {
  if (wifiConnected) {
    Blynk.run();
  }

  // ===== Read Sensors =====
  float temp = dht.readTemperature();
  int mq135Analog = analogRead(MQ135_A);
  int mq135Digital = digitalRead(MQ135_D);

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  bool vibration = digitalRead(VIBRATION_PIN) == HIGH;
  delay(10); // debounce
  bool vibrationDetected = vibration;

  // ===== ALERT CONDITIONS =====
  bool alert = false;
  if ((temp > 40 && !isnan(temp)) || mq135Digital == LOW || distance < 100 || vibrationDetected) {
    alert = true;
  }

  digitalWrite(BUZZER_PIN, alert ? HIGH : LOW);
  digitalWrite(LED_PIN, alert ? HIGH : LOW);
  digitalWrite(LASER_PIN, alert ? HIGH : LOW);

  // ===== LCD Display =====
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(isnan(temp) ? 0 : temp, 1);
  lcd.print((char)223); // degree symbol
  lcd.print(mq135Digital == LOW ? " AQ:BAD" : " AQ:Good");

  lcd.setCursor(0, 1);
  lcd.print("D:");
  lcd.print(distance, 0);
  lcd.print(" AQ:");
  lcd.print(mq135Analog);

  byte signal_up[8] = {
    0b00000,0b00001,0b00011,0b00111,0b01111,0b11111,0b11111,0b00000
  };
  byte signal_down[8] = {
    0b10001,0b01010,0b00100,0b01010,0b10001,0b00000,0b00000,0b00000
  };

  lcd.createChar(0, signal_up);
  lcd.createChar(1, signal_down);

  if (wifiConnected) {
    lcd.setCursor(15, 1);
    lcd.write(byte(0));
  } else {
    lcd.setCursor(15, 1);
    lcd.write(byte(1));
  }

  // ===== Serial Debug =====
  Serial.print("Temp:");
  Serial.print(temp);
  Serial.print(" AQ:");
  Serial.print(mq135Analog);
  Serial.print(" Dist:");
  Serial.print(distance);
  Serial.print(" Vib:");
  Serial.print(vibrationDetected);
  Serial.print(" Alert:");
  Serial.println(alert);

  // ===== Send to Blynk =====
  if (wifiConnected) {
    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, mq135Analog);
    Blynk.virtualWrite(V2, distance);
    Blynk.virtualWrite(V3, vibrationDetected ? 1 : 0);
    Blynk.virtualWrite(V4, alert ? 1 : 0);
    Blynk.virtualWrite(V5, wifiConnected ? "Connected" : "Not Connected");
  }

  delay(1000);
}
