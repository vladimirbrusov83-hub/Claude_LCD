/*
  Claude Code Monitor — Arduino Firmware v2.0
  Hardware: Arduino Uno + SainSmart LCD2004 (20×4 I2C)

  IDLE SCREEN:
    Row 0:  "    CLAUDE CODE     "
    Row 1:  "      MONITOR       "
    Row 2:  [mascot walks left/right, freezes, runs offscreen]
    Row 3:  CPU / MEM sysinfo

  STATUS SCREEN (Claude is working):
    Row 0:  [mascot] CLAUDE CODE
    Row 1:  <status word>...
    Row 2:  <detail / filename>
    Row 3:  CPU / MEM sysinfo

  MASCOT: 2 chars wide (10×8 px). 4 char slots used (2 walk frames).
  BEHAVIORS: WALKING · FROZEN · RUNAWAY (runs off screen, returns)
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);

// ── Mascot custom chars (4 slots) ─────────────────────────────────────
//   Frame A — legs down (normal stance)
byte ML_A[8] = { B00000, B01111, B01011, B11111, B11111, B01111, B01010, B01010 };
byte MR_A[8] = { B00000, B11100, B10100, B11110, B11110, B11100, B10100, B10100 };
//   Frame B — alternate legs (outer feet lifted)
byte ML_B[8] = { B00000, B01111, B01011, B11111, B11111, B01111, B01010, B00010 };
byte MR_B[8] = { B00000, B11100, B10100, B11110, B11110, B11100, B10100, B10000 };

#define CH_ML_A  0
#define CH_MR_A  1
#define CH_ML_B  2
#define CH_MR_B  3

// ── Serial / display state ─────────────────────────────────────────────
String  currentStatus = "";
String  currentDetail = "";
String  sysInfo       = "";
bool    idleMode      = true;

unsigned long lastDotTime = 0;
int dotCount = 0;

// ── Mascot animation state ─────────────────────────────────────────────
int  mascotCol    = 9;   // left char LCD column (0–18)
int  mascotDir    = 1;   // +1 = right, -1 = left
int  mascotFrame  = 0;   // 0 = A, 1 = B

enum Behavior { WALKING, FROZEN, RUNAWAY };
Behavior behavior = WALKING;

bool          offscreen   = false;
unsigned long lastMove    = 0;
unsigned long behaviorEnd = 0;
unsigned long returnAt    = 0;   // millis() when mascot re-appears after RUNAWAY

#define WALK_MS   280
#define RUN_MS     55
#define DOT_MS    400


// ═══════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  lcd.createChar(CH_ML_A, ML_A);
  lcd.createChar(CH_MR_A, MR_A);
  lcd.createChar(CH_ML_B, ML_B);
  lcd.createChar(CH_MR_B, MR_B);

  // Splash
  lcd.clear();
  lcd.setCursor(3, 1); lcd.print("[ CLAUDE CODE ]");
  lcd.setCursor(5, 2); lcd.print("MONITOR v2.0");
  delay(2000);

  randomSeed(analogRead(0));
  showIdle();
}

// ═══════════════════════════════════════════════════════════════════════
void loop() {

  // ── Serial input ──────────────────────────────────────────────────
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.startsWith("STATUS:")) {
      currentStatus = line.substring(7);
      currentDetail = "";
      dotCount      = 0;
      idleMode      = false;
      showStatus();

    } else if (line.startsWith("DETAIL:")) {
      currentDetail = line.substring(7);
      if ((int)currentDetail.length() > 18)
        currentDetail = currentDetail.substring(0, 15) + "...";
      if (!idleMode) showStatus();

    } else if (line == "CLEAR") {
      currentStatus = "";
      currentDetail = "";
      if (!idleMode) { idleMode = true; showIdle(); }

    } else if (line.startsWith("SYSINFO:")) {
      sysInfo = line.substring(8);
      updateSysInfo();
    }
  }

  // ── Dot animation (status mode only) ─────────────────────────────
  if (!idleMode && currentStatus.length() > 0
      && millis() - lastDotTime > DOT_MS) {
    lastDotTime = millis();
    dotCount    = (dotCount + 1) % 4;
    animateDots();
  }

  // ── Mascot idle animation ─────────────────────────────────────────
  if (idleMode) updateMascot();
}


// ═══════════════════════════════════════════════════════════════════════
// SCREENS

void showIdle() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("    CLAUDE CODE     ");
  lcd.setCursor(0, 1); lcd.print("      MONITOR       ");
  // Row 2 drawn by updateMascot()
  updateSysInfo();

  // Reset mascot
  mascotCol    = 9;
  mascotDir    = 1;
  mascotFrame  = 0;
  offscreen    = false;
  behavior     = WALKING;
  behaviorEnd  = millis() + random(1000, 3000);

  drawMascot(mascotCol, mascotFrame);
}

void showStatus() {
  lcd.clear();

  // Row 0: mascot (static, frame A) + label
  lcd.setCursor(0, 0);
  lcd.write(CH_ML_A); lcd.write(CH_MR_A);
  lcd.print(" CLAUDE CODE");

  // Row 1: status centered with dots
  int sLen   = currentStatus.length();
  int sStart = max(0, (20 - sLen - 3) / 2);
  lcd.setCursor(0, 1); lcd.print("                    ");
  lcd.setCursor(sStart, 1); lcd.print(currentStatus);

  // Row 2: detail
  lcd.setCursor(0, 2); lcd.print("                    ");
  if (currentDetail.length() > 0) {
    lcd.setCursor(1, 2); lcd.print(currentDetail);
  }

  updateSysInfo();
  animateDots();
}

void updateSysInfo() {
  lcd.setCursor(0, 3);
  String s = sysInfo;
  while ((int)s.length() < 20) s += " ";
  if ((int)s.length() > 20) s = s.substring(0, 20);
  lcd.print(s);
}

void animateDots() {
  if (currentStatus.length() == 0) return;
  int sLen   = currentStatus.length();
  int sStart = max(0, (20 - sLen - 3) / 2);
  int dPos   = sStart + sLen;
  lcd.setCursor(dPos, 1); lcd.print("   ");
  lcd.setCursor(dPos, 1);
  for (int i = 0; i < dotCount; i++) lcd.print(".");
}


// ═══════════════════════════════════════════════════════════════════════
// MASCOT ANIMATION

void drawMascot(int col, int frame) {
  if (col < 0 || col + 1 > 19) return;
  lcd.setCursor(col, 2);
  if (frame == 0) { lcd.write(CH_ML_A); lcd.write(CH_MR_A); }
  else            { lcd.write(CH_ML_B); lcd.write(CH_MR_B); }
}

void eraseMascot(int col) {
  if (col < 0 || col + 1 > 19) return;
  lcd.setCursor(col, 2);
  lcd.print("  ");
}

void pickBehavior() {
  unsigned long now = millis();
  int r = random(100);

  if (r < 55) {
    behavior = WALKING;
    if (random(3) == 0) mascotDir = -mascotDir;   // occasional direction flip
    behaviorEnd = now + random(800, 4000);

  } else if (r < 80) {
    behavior = FROZEN;
    behaviorEnd = now + random(800, 2800);

  } else {
    behavior = RUNAWAY;
    behaviorEnd = now + 200000UL;  // will end when offscreen logic fires
  }
}

void updateMascot() {
  unsigned long now = millis();

  // ── Waiting off screen ──────────────────────────────────────────────
  if (offscreen) {
    if (now >= returnAt) {
      offscreen   = false;
      // Appear from opposite edge, same direction (wrap-around feel)
      mascotCol   = (mascotDir > 0) ? 0 : 18;
      behavior    = WALKING;
      behaviorEnd = now + random(1000, 3000);
      drawMascot(mascotCol, mascotFrame);
    }
    return;
  }

  // ── State transition ────────────────────────────────────────────────
  if (now >= behaviorEnd) pickBehavior();

  // ── Step timer ──────────────────────────────────────────────────────
  unsigned long interval = (behavior == RUNAWAY) ? RUN_MS : WALK_MS;
  if (now - lastMove < interval) return;
  lastMove = now;

  // ── Frozen: hold frame A, no movement ───────────────────────────────
  if (behavior == FROZEN) {
    drawMascot(mascotCol, 0);
    return;
  }

  // ── Move ────────────────────────────────────────────────────────────
  eraseMascot(mascotCol);
  mascotCol  += mascotDir;
  mascotFrame = 1 - mascotFrame;

  // ── Edge handling ───────────────────────────────────────────────────
  if (mascotCol < 0 || mascotCol > 18) {
    if (behavior == RUNAWAY) {
      offscreen = true;
      returnAt  = now + random(2000, 3500);
    } else {
      // Bounce
      mascotDir  = -mascotDir;
      mascotCol += 2 * mascotDir;
      drawMascot(mascotCol, mascotFrame);
    }
    return;
  }

  drawMascot(mascotCol, mascotFrame);
}
