#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

// ===============================
// PUSH BUTTON
// ===============================
const int PB_JAM = 32;
const int PB_MENIT = 19;

// Edge detection state
bool jamLastState    = HIGH;
bool menitLastState  = HIGH;

// Cooldown agar tekanan tidak double
unsigned long lastJamMs    = 0;
unsigned long lastMenitMs  = 0;
const unsigned long PRESS_COOLDOWN = 300;  // 300ms minimum antar tekanan

// LCD update interval — besarin agar I2C tidak sibuk
unsigned long lastDisplayMs = 0;
const unsigned long DISPLAY_INTERVAL = 300;  // update LCD setiap 300ms

// ===============================
// SETUP
// ===============================
void setup() {

  Serial.begin(115200);

  // I2C
  Wire.begin(21, 22);

  // Push Button
  pinMode(PB_JAM, INPUT_PULLUP);
  pinMode(PB_MENIT, INPUT_PULLUP);

  // LCD
  lcd.init();
  lcd.backlight();

  // RTC
  if (!rtc.begin()) {

    Serial.println("RTC DS1307 tidak terdeteksi");

    lcd.setCursor(0, 0);
    lcd.print("RTC not found");

    while (1) {
      delay(10);
    }
  }

  // ===============================
  // ATUR WAKTU AWAL
  // ===============================
  // Jalankan sekali untuk mengatur RTC.
  // Setelah itu komentari baris ini.

  rtc.adjust(DateTime(2026, 9, 21, 10, 0, 0));

  lcd.setCursor(0, 0);
  lcd.print("RTC DS1307 Demo");

  delay(1500);
  lcd.clear();
}

// ===============================
// TAMPILKAN WAKTU KE LCD
// ===============================
void updateDisplay() {
  DateTime now = rtc.now();

  // Tanggal
  lcd.setCursor(0, 0);

  lcd.print(now.year());
  lcd.print('/');

  if (now.month() < 10) {
    lcd.print('0');
  }

  lcd.print(now.month());
  lcd.print('/');

  if (now.day() < 10) {
    lcd.print('0');
  }

  lcd.print(now.day());
  lcd.print("   ");

  // Jam
  lcd.setCursor(0, 1);

  if (now.hour() < 10) {
    lcd.print('0');
  }

  lcd.print(now.hour());
  lcd.print(':');

  if (now.minute() < 10) {
    lcd.print('0');
  }

  lcd.print(now.minute());
  lcd.print(':');

  if (now.second() < 10) {
    lcd.print('0');
  }

  lcd.print(now.second());
  lcd.print("   ");
}

// ===============================
// LOOP
// ===============================
void loop() {

  unsigned long nowMs = millis();

  // ==========================================
  // 1. BACA TOMBOL PERTAMA & SECEPAT MUNGKIN
  //    (sebelum I2C operasi apapun)
  // ==========================================

  bool jamReading    = digitalRead(PB_JAM);
  bool menitReading  = digitalRead(PB_MENIT);

  // PB JAM: deteksi falling edge (HIGH → LOW)
  if (jamLastState == HIGH && jamReading == LOW) {
    if (nowMs - lastJamMs >= PRESS_COOLDOWN) {

      DateTime now = rtc.now();
      now = now + TimeSpan(0, 1, 0, 0);
      rtc.adjust(now);

      Serial.println("PB JAM -> +1 JAM");
      lastJamMs = nowMs;
    }
  }
  jamLastState = jamReading;

  // PB MENIT: deteksi falling edge (HIGH → LOW)
  if (menitLastState == HIGH && menitReading == LOW) {
    if (nowMs - lastMenitMs >= PRESS_COOLDOWN) {

      DateTime now = rtc.now();
      now = now + TimeSpan(0, 0, 1, 0);
      rtc.adjust(now);

      Serial.println("PB MENIT -> +1 MENIT");
      lastMenitMs = nowMs;
    }
  }
  menitLastState = menitReading;

  // ==========================================
  // 2. TAMPILKAN WAKTU (jarang agar I2C tidak sibuk)
  // ==========================================
  if (nowMs - lastDisplayMs >= DISPLAY_INTERVAL) {
    updateDisplay();
    lastDisplayMs = nowMs;
  }
}
