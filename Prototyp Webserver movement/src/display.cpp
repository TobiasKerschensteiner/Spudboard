#include <Arduino.h>
//#include <Adafruit_GC9A01A.h>
//#include <Adafruit_GFX.h>
//#include <FS.h>
//#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <math.h>


/*
// Display-Pins definieren
#define TFT_CS   22  // Chip Select Pin
#define TFT_RST  4  // Reset Pin
#define TFT_DC   16  // Data/Command Pin
#define TFT_DIN  23
#define TFT_CLK  18
  */

TFT_eSPI tft = TFT_eSPI();  // Initialisiere die Bibliothek für das TFT-Display

unsigned long previousMillis = 0;  // Speichert den letzten Zeitpunkt, zu dem das Display aktualisiert wurde
const long interval = 1000;  // Intervall für die Aktualisierung der Uhrzeit in Millisekunden (1000 ms = 1 Sekunde)

int startMinutes = 0;  // Startminuten für den Countdown
int startSeconds = 30;  // Startsekunden für den Countdown
int totalSeconds = startMinutes * 60 + startSeconds;  // Gesamtzeit des Countdowns in Sekunden
int remainingSeconds = totalSeconds;  // Verbleibende Zeit des Countdowns in Sekunden

bool isCharging = true; // Setzen Sie diese Variable entsprechend dem Ladezustand des Akkus
bool wlanverbunden = true; // setzten endprechen ob mit WLAn verbunden oder nicht 

void setup(void) {
  tft.init();  // Initialisiere das TFT-Display
  tft.setTextDatum(MC_DATUM); // Setze den Textausrichtungspunkt in die Mitte des Displays
  tft.setTextSize(4);  // Setze die Textgröße
  tft.setRotation(1);  // Richtige Ausrichtung für den Füll-Effekt
  tft.fillScreen(TFT_BLACK);
}


//WLANsymbol
void drawWLANSymbol() {
  int centerX = tft.width() / 2;
  int topY = 10; // Abstand vom oberen Rand des Bildschirms für das WLANsymbol
  int radius = 5;  // radius kreis
  

  // Zeichne das größere Ladesymbol nur, wenn isCharging true ist
  if (wlanverbunden) {
      tft.fillCircle(centerX - radius /2,topY, radius, TFT_PINK);
      tft.drawCircle(centerX - radius / 2, topY, radius, TFT_PINK);

  }
}

//WLAN
void drawWiFiDetails() {
  tft.fillScreen(TFT_BLACK);
    // Definiere die Position für die WLAN-Details
    int startX = 120;  // Abstand vom linken Rand des Bildschirms
    int startY = 60;  // Abstand vom oberen Rand des Bildschirms
    int lineHeight = 30;  // Höhe zwischen den Zeilen
    tft.setTextSize(2);

    // Lösche den Bereich, wo die WLAN-Details erscheinen
    tft.fillRect(startX, startY, tft.width() - 2 * startX, 2 * lineHeight + 5, TFT_BLACK);

    // Setze die Textfarbe
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Weißer Text mit schwarzem Hintergrund

    // Zeichne "WLAN:" Text
    tft.drawString("WLAN:", startX, startY, 2);  // Verwenden Sie die passende Schriftgröße

    // Zeichne die maskierte ip
    tft.drawString("XXXXX", startX, startY + lineHeight, 2);  // Verwenden Sie die passende Schriftgröße

    // Zeichne das Passwort text
    tft.drawString("Password:", startX, startY + 2 * lineHeight, 2);  // Verwenden Sie die passende Schriftgröße

    // Zeichne das maskierte Passwort
    tft.drawString("XXXXXXXXX", startX, startY + 3 * lineHeight, 2);  // Verwenden Sie die passende Schriftgröße
}


//Ladesymbol
void drawChargingSymbol() {
  tft.setTextSize(4);
    int centerX = tft.width() / 2;
    int topY = 10; // Abstand vom oberen Rand des Bildschirms für das Ladesymbol
    int rectWidth = 20;  // Breite des größeren Rechtecksymbols
    int rectHeight = 10;  // Höhe des größeren Rechtecksymbols
    int smallRectWidth = 2;  // Breite des kleineren Rechtecksymbols
    int smallRectHeight = 4;  // Höhe des kleineren Rechtecksymbols
    int spaceBetween = 1;  // Abstand zwischen den beiden Rechtecken


    // Lösche den Bereich, wo die Ladesymbole erscheinen
    tft.fillRect(centerX - rectWidth / 2 - 1, topY - 1, rectWidth + smallRectWidth + spaceBetween + 3, rectHeight + 2, TFT_BLACK);

    // Zeichne das größere Ladesymbol nur, wenn isCharging true ist
    if (isCharging) {
        tft.drawRect(centerX - rectWidth / 2, topY, rectWidth, rectHeight, TFT_YELLOW);
        // Zeichne das kleinere Rechteck rechts neben dem größeren Rechteck
        tft.drawRect(centerX - rectWidth / 2 + rectWidth + spaceBetween, topY + (rectHeight - smallRectHeight) / 2, smallRectWidth, smallRectHeight, TFT_YELLOW);
    }
}

//Akkuanzeige
void drawBatteryLevel(float batteryLevel) {

  tft.setTextSize(4);
    // Aktualisiere den Hintergrund basierend auf dem Akkustand
    tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);  // Lösche zuerst den gesamten Bildschirm
    int fillHeight = tft.height() * batteryLevel;  // Berechne die Füllhöhe basierend auf dem Akkustand
    tft.fillRect(0, tft.height() - fillHeight, tft.width(), fillHeight, TFT_GREEN);  // Fülle den unteren Bereich des Bildschirms

    // Definiere den Bereich für den Prozenttext
    int textX = tft.width() / 2;
    int textY = tft.height() / 2;

    // Bereite den Akkustand als Text vor
    char batteryStr[10];
    sprintf(batteryStr, "%d%%", (int)(batteryLevel * 100));

    // Setze die Textfarbe und zeichne den Akkustand
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Weißer Text mit schwarzem Hintergrund
    tft.drawString(batteryStr, textX, textY, 2);  // Stellen Sie sicher, dass die richtige Schriftgröße verwendet wird
}


void drawFilledSegment(int centerX, int centerY, int radius, float startAngle, float endAngle, uint32_t color) {
  // Definiere die Segmentgröße (in Grad)
  float segmentSize = 5.0;

  for (float angle = startAngle; angle < endAngle; angle += segmentSize) {
    float angleRad = angle * DEG_TO_RAD; // Umrechnung in Radiant
    float nextAngleRad = (angle + segmentSize) * DEG_TO_RAD; // Nächster Winkel in Radiant

    // Berechne die Koordinaten der Eckpunkte des Segments
    int x1 = centerX + radius * cos(angleRad);
    int y1 = centerY + radius * sin(angleRad);
    int x2 = centerX + radius * cos(nextAngleRad);
    int y2 = centerY + radius * sin(nextAngleRad);

    // Zeichne das Segment als Dreieck
    tft.fillTriangle(centerX, centerY, x1, y1, x2, y2, color);
  }
}

//Hintergrund Timer anzeige
void updateTimerDisplay() {
  tft.setTextSize(4);
  // Berechne den Winkel für den Füll-Effekt basierend auf der verbleibenden Zeit
  float angle = 360.0 * (1.0 - (float)remainingSeconds / (float)totalSeconds);

  // Lösche den Bildschirm
  tft.fillScreen(TFT_BLACK);

  // Zeichne den gefüllten Sektor
  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int radius = min(tft.width(), tft.height()) / 2;
  drawFilledSegment(centerX, centerY, radius, 0, angle, TFT_CYAN);

  // Bereite die Zeit als Zeichenkette vor und zeichne sie
  int minutes = remainingSeconds / 60;
  int seconds = remainingSeconds % 60;
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);  // Formatieren der Zeit im Format mm:ss
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Textfarbe auf Weiß mit schwarzem Hintergrund setzen
  tft.drawString(timeStr, centerX, centerY); // Zeichne die Zeit in der Mitte des Bildschirms
}

//Funktion Timer
void time() {
  unsigned long currentMillis = millis();  // Aktuelle Zeit seit Programmstart in Millisekunden

  if (currentMillis - previousMillis >= interval) {  // Überprüfe, ob das festgelegte Intervall vergangen ist
    previousMillis = currentMillis;  // Aktualisiere die letzte Aktualisierungszeit

    // Dekrementiere die verbleibenden Sekunden jede Sekunde
    if (remainingSeconds > 0) {
      remainingSeconds--;
      updateTimerDisplay();  // Aktualisiere die Anzeige mit der neuen Zeit und dem neuen Hintergrund
    } 
  }
}

void loop() {
  static unsigned long lastChangeMillis = 0;  // Speichert den Zeitpunkt des letzten Funktionswechsels
  static int state = 0;  // Zustandsvariable, die bestimmt, welche Funktion ausgeführt wird

  unsigned long currentMillis = millis();  // Aktuelle Zeit seit dem Programmstart in Millisekunden

  // Überprüfe, ob 5 Sekunden vergangen sind
  if (currentMillis - lastChangeMillis >= 5000) {
    lastChangeMillis = currentMillis;  // Aktualisiere die letzte Wechselzeit

    // Wechsel zwischen den Zuständen
    state = (state + 1) % 4;  // Erhöhe den Zustand und wende Modulo an, um im Bereich [0,3] zu bleiben
  }

  // Führe die Funktion basierend auf dem aktuellen Zustand aus
  switch (state) {
    case 0:
      time();  // Aktualisiere und zeige den Timer
      break;
    case 1: {
      float batteryLevel = 0.50;  // Beispiel: Akkustand
      drawBatteryLevel(batteryLevel);  // Zeichne die Akkuanzeige
      drawChargingSymbol();
      delay(5000);
      break;
    }
    case 2:
      drawWiFiDetails();  // Zeichne die WLAN-Details
      drawWLANSymbol();  // Zeichne das WLAN-Symbol
      delay(5000);
      break;
  }
}






