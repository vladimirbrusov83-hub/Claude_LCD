/*
  Claude Code Status Monitor — Arduino Firmware v1.2
  Hardware: Arduino Uno + 20x4 LCD with I2C backpack

  Features:
    - Claude Code tool status with animated dots
    - CPU/RAM usage on row 3, updated every few seconds

  WIRING:
    LCD GND → Arduino GND
    LCD VCC → Arduino 5V
    LCD SDA → Arduino A4
    LCD SCL → Arduino A5

  LIBRARY:
    Arduino IDE → Tools → Manage Libraries → "LiquidCrystal I2C" by Frank de Brabander
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);

// ── Custom chars: Claude Code diamond mascot (2×2 tile) ──────────────
byte DIAMOND_TL[8] = { B00000, B00001, B00011, B00111, B01111, B11111, B11111, B11111 };
byte DIAMOND_TR[8] = { B00000, B10000, B11000, B11100, B11110, B11111, B11111, B11111 };
byte DIAMOND_BL[8] = { B11111, B11111, B11111, B01111, B00111, B00011, B00001, B00000 };
byte DIAMOND_BR[8] = { B11111, B11111, B11111, B11110, B11100, B11000, B10000, B00000 };
byte FULL_BLOCK[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };

#define CH_TL    0
#define CH_TR    1
#define CH_BL    2
#define CH_BR    3
#define CH_BLOCK 4

// ── Normal mode state ─────────────────────────────────────────────────
String currentStatus = "";
String currentDetail = "";
String sysInfo       = "";
unsigned long lastDotTime = 0;
int dotCount = 0;


// ═══════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  loadDiamondChars();

  // Splash screen
  lcd.setCursor(3, 1); lcd.print("[ CLAUDE CODE ]");
  lcd.setCursor(5, 2); lcd.print("MONITOR v1.2");
  delay(2500);
  lcd.clear();

  showIdle();
}

// ═══════════════════════════════════════════════════════════════════════
void loop() {

  // ── Serial input ────────────────────────────────────────────────────
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.startsWith("STATUS:")) {
      currentStatus = line.substring(7);
      currentDetail = "";
      dotCount = 0;
      showStatus();

    } else if (line.startsWith("DETAIL:")) {
      currentDetail = line.substring(7);
      if ((int)currentDetail.length() > 18)
        currentDetail = currentDetail.substring(0, 15) + "...";
      if (currentStatus.length() > 0) showStatus();

    } else if (line == "CLEAR") {
      currentStatus = "";
      currentDetail = "";
      showIdle();

    } else if (line.startsWith("SYSINFO:")) {
      sysInfo = line.substring(8);
      updateSysInfo();
    }
  }

  // ── Dot animation ───────────────────────────────────────────────────
  if (currentStatus.length() > 0 && millis() - lastDotTime > 400) {
    lastDotTime = millis();
    dotCount = (dotCount + 1) % 4;
    animateDots();
  }
}


// ═══════════════════════════════════════════════════════════════════════
// Normal display

void loadDiamondChars() {
  lcd.createChar(CH_TL, DIAMOND_TL);
  lcd.createChar(CH_TR, DIAMOND_TR);
  lcd.createChar(CH_BL, DIAMOND_BL);
  lcd.createChar(CH_BR, DIAMOND_BR);
  lcd.createChar(CH_BLOCK, FULL_BLOCK);
}

//  Row 0: ────────────────────
//  Row 1:    ◆◆ Claude Code
//  Row 2:    ◆◆   Monitor
//  Row 3:  CPU: 8%  MEM: 45%
void showIdle() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("--------------------");
  lcd.setCursor(3, 1); lcd.write(CH_TL); lcd.write(CH_TR); lcd.print(" Claude Code");
  lcd.setCursor(3, 2); lcd.write(CH_BL); lcd.write(CH_BR); lcd.print("   Monitor");
  updateSysInfo();
}

//  Row 0:  ◆◆ CLAUDE CODE
//  Row 1:     <status>...
//  Row 2:     <detail>
//  Row 3:  CPU: 8%  MEM: 45%
void showStatus() {
  lcd.clear();

  // Row 0 — header with diamond mascot
  lcd.setCursor(0, 0);
  lcd.write(CH_TL); lcd.write(CH_TR);
  lcd.print(" CLAUDE CODE");

  // Row 1 — status word, centered, with room for animated dots
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  int statusLen   = currentStatus.length();
  int statusStart = (20 - statusLen - 3) / 2;
  if (statusStart < 0) statusStart = 0;
  lcd.setCursor(statusStart, 1);
  lcd.print(currentStatus);

  // Row 2 — detail / filename
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  if (currentDetail.length() > 0) {
    lcd.setCursor(1, 2);
    lcd.print(currentDetail);
  }

  updateSysInfo();
  animateDots();
}

// Write sysinfo string to row 3 (padded to 20 chars)
void updateSysInfo() {
  lcd.setCursor(0, 3);
  String padded = sysInfo;
  while ((int)padded.length() < 20) padded += " ";
  if ((int)padded.length() > 20) padded = padded.substring(0, 20);
  lcd.print(padded);
}

void animateDots() {
  if (currentStatus.length() == 0) return;
  int statusLen   = currentStatus.length();
  int statusStart = (20 - statusLen - 3) / 2;
  if (statusStart < 0) statusStart = 0;
  int dotsPos = statusStart + statusLen;
  lcd.setCursor(dotsPos, 1); lcd.print("   ");
  lcd.setCursor(dotsPos, 1);
  for (int i = 0; i < dotCount; i++) lcd.print(".");
}
