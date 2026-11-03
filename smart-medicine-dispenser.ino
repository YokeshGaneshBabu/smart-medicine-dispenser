#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include "DFRobotDFPlayerMini.h"
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>

// ─── WiFi Credentials ──────────────────────────
const char* ssid     = "Device_Name";
const char* password = "Password";

// ─── Web Server ────────────────────────────────
WebServer server(80);

// ─── Pill Status ───────────────────────────────
bool pillTaken[4] = {false, false, false, false};
const char* pillLabels[4] = {
  "Morning",
  "Afternoon (Before Food)",
  "Afternoon (After Food)",
  "Night"
};
const char* pillIcons[4] = {"Morning", "Noon BF", "Noon AF", "Night"};

// ─── LCD ───────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ─── RTC ───────────────────────────────────────
RTC_DS3231 rtc;

// ─── DFPlayer ──────────────────────────────────
HardwareSerial mySerial(2);
DFRobotDFPlayerMini myDFPlayer;

// ─── Servos ────────────────────────────────────
Servo servoMorning;
Servo servoAfternoon;
Servo servoNight;

#define SERVO_MORNING_PIN    13
#define SERVO_AFTERNOON_PIN  12
#define SERVO_NIGHT_PIN      14

// ─── IR Sensors ────────────────────────────────
#define IR_MORNING           34
#define IR_AFTERNOON         35
#define IR_NIGHT             32

// ─── LEDs ──────────────────────────────────────
#define LED_BEFORE_FOOD      26
#define LED_AFTER_FOOD       25

// ─── Angles & Timing ───────────────────────────
#define LID_BEFORE_ANGLE     45
#define LID_AFTER_ANGLE      45
#define LID_CLOSED_ANGLE      0
#define AUDIO_TRACK          5
#define AUDIO_REPEAT_MS      20000
#define HAND_CLOSE_DELAY     5000
#define NEXT_OPEN_DELAY      5000

// ─── Get current time string HH:MM (24hr) ──────
String getTimeStr() {
  DateTime now = rtc.now();
  char buf[6];
  // hour() returns 0-23 (24hr), sprintf with %02d gives correct zero-padding
  sprintf(buf, "%02d:%02d", now.hour(), now.minute());
  return String(buf);
}

// ─── Get current date string DD/MM/YYYY ────────
String getDateStr() {
  DateTime now = rtc.now();
  char buf[11];
  sprintf(buf, "%02d/%02d/%04d", now.day(), now.month(), now.year());
  return String(buf);
}

// ─── Web Server Handler ────────────────────────
void handleRoot() {
  int takenCount = 0;
  for (int i = 0; i < 4; i++) if (pillTaken[i]) takenCount++;
  int missedCount = 4 - takenCount;

  String timeStr = getDateStr() + " " + getTimeStr();

  // Build HTML in small chunks to avoid long compile from big raw strings
  String html = "";

  html += "<!DOCTYPE html><html lang='en'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='10'>";
  html += "<title>Smart Medicine Box</title>";
  html += "<style>";
  html += "*{margin:0;padding:0;box-sizing:border-box;}";
  html += "body{font-family:'Segoe UI',sans-serif;background:#0f172a;color:#e2e8f0;min-height:100vh;padding:24px 16px;}";
  html += ".header{text-align:center;margin-bottom:24px;}";
  html += ".header h1{font-size:1.6rem;font-weight:700;color:#38bdf8;}";
  html += ".header .sub{font-size:0.8rem;color:#64748b;margin-top:4px;}";
  html += ".timebadge{display:inline-block;background:#1e293b;border:1px solid #334155;border-radius:20px;padding:5px 16px;font-size:0.78rem;color:#94a3b8;margin-top:10px;}";
  html += ".summary{display:flex;justify-content:center;gap:12px;margin-bottom:22px;flex-wrap:wrap;}";
  html += ".scard{background:#1e293b;border-radius:12px;padding:14px 22px;text-align:center;border:1px solid #334155;min-width:100px;}";
  html += ".scard .num{font-size:2rem;font-weight:800;line-height:1;}";
  html += ".scard .lbl{font-size:0.7rem;color:#64748b;margin-top:4px;text-transform:uppercase;letter-spacing:0.5px;}";
  html += ".blue{color:#38bdf8;}.green{color:#22c55e;}.red{color:#ef4444;}";
  html += ".cards{display:grid;grid-template-columns:1fr 1fr;gap:14px;max-width:520px;margin:0 auto;}";
  html += "@media(max-width:400px){.cards{grid-template-columns:1fr;}}";
  html += ".card{background:#1e293b;border-radius:14px;padding:18px 16px;border:1.5px solid #334155;}";
  html += ".card.taken{border-color:#16a34a;}";
  html += ".card.missed{border-color:#dc2626;}";
  html += ".slabel{font-size:0.7rem;color:#64748b;text-transform:uppercase;letter-spacing:0.6px;margin-bottom:5px;}";
  html += ".sname{font-size:0.96rem;font-weight:600;color:#e2e8f0;margin-bottom:10px;}";
  html += ".pill{display:inline-flex;align-items:center;border-radius:20px;padding:4px 12px;font-size:0.78rem;font-weight:600;}";
  html += ".pill.ok{background:#14532d55;color:#4ade80;border:1px solid #16a34a;}";
  html += ".pill.no{background:#7f1d1d55;color:#f87171;border:1px solid #dc2626;}";
  html += ".dot{width:9px;height:9px;border-radius:50%;display:inline-block;margin-right:6px;}";
  html += ".dot.g{background:#22c55e;box-shadow:0 0 5px #22c55e88;}";
  html += ".dot.r{background:#ef4444;box-shadow:0 0 5px #ef444488;animation:blink 1.2s infinite;}";
  html += "@keyframes blink{0%,100%{opacity:1;}50%{opacity:0.25;}}";
  html += ".footer{text-align:center;margin-top:24px;font-size:0.7rem;color:#334155;}";
  html += ".note{text-align:center;font-size:0.7rem;color:#475569;margin-top:14px;}";
  html += "</style></head><body>";

  // Header
  html += "<div class='header'>";
  html += "<h1>&#128138; Smart Medicine Box</h1>";
  html += "<div class='sub'>Real-time pill tracking</div>";
  html += "<div class='timebadge'>&#128336; ";
  html += timeStr;
  html += " IST</div></div>";

  // Summary
  html += "<div class='summary'>";
  html += "<div class='scard'><div class='num blue'>4</div><div class='lbl'>Total</div></div>";
  html += "<div class='scard'><div class='num green'>";
  html += takenCount;
  html += "</div><div class='lbl'>Taken</div></div>";
  html += "<div class='scard'><div class='num red'>";
  html += missedCount;
  html += "</div><div class='lbl'>Missed</div></div>";
  html += "</div>";

  // Cards
  html += "<div class='cards'>";
  for (int i = 0; i < 4; i++) {
    bool taken = pillTaken[i];
    html += "<div class='card ";
    html += taken ? "taken" : "missed";
    html += "'><div class='slabel'>Slot ";
    html += (i + 1);
    html += " &mdash; ";
    html += pillIcons[i];
    html += "</div><div class='sname'>";
    html += pillLabels[i];
    html += "</div>";
    html += "<span class='pill ";
    html += taken ? "ok" : "no";
    html += "'><span class='dot ";
    html += taken ? "g" : "r";
    html += "'></span>";
    html += taken ? "Pill Taken &#10003;" : "Pill Not Taken";
    html += "</span></div>";
  }
  html += "</div>";

  html += "<div class='note'>&#8635; Auto-refreshes every 10 seconds</div>";
  html += "<div class='footer'>Smart Medicine Dispenser &nbsp;|&nbsp; ESP32</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ─── LCD Helper ────────────────────────────────
void lcdMessage(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}

// ─── LCD Clock — HH:MM only, no seconds ────────
void lcdClock() {
  DateTime now = rtc.now();
  char dateBuf[17], timeBuf[17];
  sprintf(dateBuf, "%02d/%02d/%04d", now.day(), now.month(), now.year());
  // Use now.hour() directly — DS3231 always returns 24hr (0–23)
  sprintf(timeBuf, "IST  %02d:%02d", now.hour(), now.minute());
  lcd.setCursor(0, 0); lcd.print(dateBuf);
  lcd.setCursor(0, 1); lcd.print(timeBuf);
}

// ─── Core Compartment Function ─────────────────
// ledMode: 0=none, 1=LED_BEFORE_FOOD, 2=LED_AFTER_FOOD
void runCompartment(Servo& servo, int irPin, int lidAngle,
                    const char* label1, const char* label2,
                    int ledMode, int pillIndex) {

  Serial.print("Opening: "); Serial.println(label2);

  // LEDs
  digitalWrite(LED_BEFORE_FOOD, ledMode == 1 ? HIGH : LOW);
  digitalWrite(LED_AFTER_FOOD,  ledMode == 2 ? HIGH : LOW);

  // Open lid
  servo.write(lidAngle);
  delay(600);

  // First reminder
  lcdMessage(label1, label2);
  myDFPlayer.play(AUDIO_TRACK);

  unsigned long lastAudio = millis();
  unsigned long startWait = millis();
  bool handDetected = false;
  #define MAX_WAIT_MS 60000

  while (!handDetected && millis() - startWait < MAX_WAIT_MS) {  // ← added time limit
    server.handleClient();

    if (digitalRead(irPin) == LOW) {
      handDetected = true;
      pillTaken[pillIndex] = true;
      Serial.println("Hand detected!");
      lcdMessage("Medicine Taken!", label2);
      delay(HAND_CLOSE_DELAY);
    } else {
      if (millis() - lastAudio >= AUDIO_REPEAT_MS) {
        lcdMessage("Take Medicine!", label2);
        myDFPlayer.play(AUDIO_TRACK);
        lastAudio = millis();
      }
    }
    delay(300);
  }

  // ← Add this block right after the while loop
  if (!handDetected) {
    Serial.println("⚠️ Timeout! Medicine not taken.");
    lcdMessage("Missed Dose!", "Closing lid...");
    delay(2000);
  }

  // Close lid
  servo.write(LID_CLOSED_ANGLE);
  delay(800);

  // LEDs off
  digitalWrite(LED_BEFORE_FOOD, LOW);
  digitalWrite(LED_AFTER_FOOD,  LOW);

  lcdMessage("Lid Closed.", "Well done!");
  delay(3000);

  lcdClock();
  delay(NEXT_OPEN_DELAY);
}

// ─── SETUP ─────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(LED_BEFORE_FOOD, OUTPUT); digitalWrite(LED_BEFORE_FOOD, LOW);
  pinMode(LED_AFTER_FOOD,  OUTPUT); digitalWrite(LED_AFTER_FOOD,  LOW);
  pinMode(IR_MORNING,   INPUT);
  pinMode(IR_AFTERNOON, INPUT);
  pinMode(IR_NIGHT,     INPUT);

  lcd.init();
  lcd.backlight();
  lcdMessage("Medicine", "Dispenser v2.0");
  delay(2000);

  if (!rtc.begin()) {
    lcdMessage("ERROR:", "RTC not found!");
    while (1);
  }
  // ── Uncomment ONE line below to set RTC time, then re-comment and re-upload ──
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // sets to compile time
  //rtc.adjust(DateTime(2026, 4, 13, 11, 0, 0));  // sets manually HH=17, MM=46
  Serial.println("RTC OK");

  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  if (!myDFPlayer.begin(mySerial)) {
    lcdMessage("ERROR:", "DFPlayer failed!");
    while (1);
  }
  myDFPlayer.volume(25);

  servoMorning.attach(SERVO_MORNING_PIN);
  servoAfternoon.attach(SERVO_AFTERNOON_PIN);
  servoNight.attach(SERVO_NIGHT_PIN);
  servoMorning.write(LID_CLOSED_ANGLE);
  servoAfternoon.write(LID_CLOSED_ANGLE);
  servoNight.write(LID_CLOSED_ANGLE);

  // WiFi
  lcdMessage("Connecting", "WiFi...");
  WiFi.begin(ssid, password);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500); tries++;
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK: " + WiFi.localIP().toString());
    lcdMessage("WiFi OK!", WiFi.localIP().toString().c_str());
  } else {
    lcdMessage("WiFi Failed.", "Offline mode");
  }
  delay(3000);

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started");

  lcdMessage("System Ready!", "Starting...");
  delay(2000);

  // ─── Sequential dispensing ──────────────────
  // 1. Morning — no LED
  runCompartment(servoMorning, IR_MORNING,
                 LID_BEFORE_ANGLE,
                 "Take Medicine!", "Morning Dose",
                 0, 0);

  // 2. Afternoon before food — LED_BEFORE_FOOD
  runCompartment(servoAfternoon, IR_AFTERNOON,
                 LID_BEFORE_ANGLE,
                 "Before Food!", "Noon-Before Food",
                 1, 1);

  // 3. Afternoon after food — LED_AFTER_FOOD
  runCompartment(servoAfternoon, IR_AFTERNOON,
                 LID_AFTER_ANGLE,
                 "After Food!", "Noon-After Food",
                 2, 2);

  // 4. Night — no LED
  runCompartment(servoNight, IR_NIGHT,
                 LID_BEFORE_ANGLE,
                 "Take Medicine!", "Night Dose",
                 0, 3);

  lcdMessage("All Doses Done!", "Great job! :)");
  Serial.println("All done!");
}

// ─── LOOP ──────────────────────────────────────
void loop() {
  server.handleClient();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    lcdClock();
    lastUpdate = millis();
  }
  delay(200);
}
