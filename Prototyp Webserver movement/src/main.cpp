/*
Hilfe:
https://cool-web.de/arduino/multi-function-shield-step-motor.htm
https://www.airspayce.com/mikem/arduino/AccelStepper/
*/


#include <SPI.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_GC9A01A.h>
#include <FS.h>
#include "SPIFFS.h"
#include <Arduino.h>
#include <AccelStepper.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <math.h>
#include <MPU6050_light.h>

// WLAN-Zugangsdaten
const char* ssid = "NOTHING1";
const char* password = "123456789";

//#define HALFSTEP 8
#ifndef DRIVER
#define DRIVER 1
#endif
// Konstanten Roboter + Geschwindigkeiten 
const float stepsPerRevolution = 10800; // Schritte für eine volle Umdrehung im Half-Step-Modus
const float SteppDegree = 30;
const int maxSpeed = 10000; //Maximale Geschwindigkeit Mopsgeschwindigkeit!!!
const int maxspeeddre = 10000; //Maximale Geschwindikeit bei den Drehungen 
const int besch = 4000; // Beschleunigung
const int desiredSpeed = 10000; // Gewünschte Geschwindigkeit in Schritten pro Sekunde
const int delays = 2000; //delay von 2 sek
const int dist = 20000; //Kleiner versatzt von 1000 schritten
const int bist = 10000; 
const int Enable = 18;
// Pinbelegung Sonstige
const int taster2 = 13; //Taster stoppen 
const int taster1 = 32; // Taster Standardroute
const int sensorPin = 23; // Pin-Nummer des Sensors
const int wisch = 19; // Pin-Nummer wischmodul
const int BL = 12; // Backlight Display
//const int SCL = 23; //gyro
//const int SDA = 22; //gyro 
int charging = 0;
bool isPreparingForHome = false;

String richtung = "oben";

// Pin-Belegung für beide Stepper-Motoren
AccelStepper stepper1(DRIVER, 4,15);  
AccelStepper stepper2(DRIVER, 5,17);

//Kalibrierung
int aktuellx=0;
int aktuelly=0;
bool xabgemessen=false;
bool yabgemessen=false;
long xabmessung=0;
long yabmessung=0;

bool isCalibratingX = false;
bool isCalibratingY = false;


//Standard
enum State {
            CHECK_BUTTON,MOVING_FORWARD,TURNING_RIGHT, TURNING_RIGHT2,
            MOVING_SHORT_DISTANCER, MOVING_SHORT_DISTANCEL,
            TURNING_LEFT, TURNING_LEFT2,
            STOPPINGUR, HOMEUR, STOPPINGOR, HOMEOR,
            PREPARE_COMING_HOME_FROM_BOTTOM,
            PREPARE_COMING_HOME_FROM_TOP,
            PREPARE_COMING_HOME_FROM_RIGHT, MOVEBACK, MOVEBACK2};
            
State currentState = MOVING_FORWARD;
bool hasTurnedRight = false;
bool turnLeftNext = false; // Flag, um zu bestimmen, ob als nächstes nach links gedreht werden soll


//Home
enum HomeState {
  TURNING_LEFT_HOME, TURNING_LEFT_HOME2, TURNING_LEFT_HOME3, TURNING_LEFT_HOME4,
  MOVING_FORWARD_HOME, MOVING_FORWARD_HOME2, MOVING_FORWARD_HOME3, MOVING_FORWARD_HOME4,
  STOPPING_HOME};
// Initialisiere den Zustand für die Home-Funktion
HomeState homeState = TURNING_LEFT_HOME;

//Home2
enum HomeState2 {
  TURNING_LEFT2_HOME, TURNING_LEFT2_HOME2, TURNING_LEFT2_HOME3, TURNING_LEFT2_HOME4,
  MOVING_FORWARD2_HOME, MOVING_FORWARD2_HOME2, MOVING_FORWARD2_HOME3, MOVING_FORWARD2_HOME4,
  STOPPING_HOME2};
// Initialisiere den Zustand für die Home-Funktion
HomeState2 homeState2 = TURNING_LEFT2_HOME;


//WEBSERVER VARIABLEN
WebServer server(80);
bool roboterInBetrieb = false;
float akkustand = 0; // Beispiel fuer einen Akkustand
int aktuellerModus = 0; // Globale Variable fuer den aktuellen Modus
int fortschritt = 0;
int verstricheneZeit;
bool aufStart = true; // 
unsigned long startZeitpunkt = 0; 
unsigned long stoppZeitpunkt = 0; 
const unsigned long wartezeit = 5000; // 5 Sekunden Wartezeit in Millisekunden
bool zurueckkehren = false; // Neue Zustandsvariable fuer "Zurueckkehren"
int modus;

MPU6050 mpu(Wire);
TFT_eSPI tft = TFT_eSPI();  // Initialisiere die Bibliothek für das TFT-Display

unsigned long timer = 0;

unsigned long previousMillis = 0;  // Speichert den letzten Zeitpunkt, zu dem das Display aktualisiert wurde
const long interval = 1000;  // Intervall für die Aktualisierung der Uhrzeit in Millisekunden (1000 ms = 1 Sekunde)

int startMinutes = 1;  // Startminuten für den Countdown
int startSeconds = 30;  // Startsekunden für den Countdown
int totalSeconds = startMinutes * 60 + startSeconds;  // Gesamtzeit des Countdowns in Sekunden
int remainingSeconds = totalSeconds;  // Verbleibende Zeit des Countdowns in Sekunden
int elapsedSeconds =0 ;

bool isCharging = true; // Setzen Sie diese Variable entsprechend dem Ladezustand des Akkus
bool isWifiConnected = true; // setzten endprechen ob mit WLAn verbunden oder nicht 
bool kalib = true; // nur zum testen da um zu schauen ob die funtnkion get muss noch an alpay angebundne werden 

//Webserver Modi mit aktueller Modus Variable und Cases

void getBattery()
{
  int Voltage = analogRead(34);

  akkustand = map(Voltage,2950,4095,0,100);
  akkustand = akkustand/100;
  //Serial.println(akkustand);

  if (akkustand >= 100)
  {
    charging = 1;
    akkustand = 100;
  }
  else 
  {
    charging = 0;
  }
  // Setze isCharging basierend auf dem Wert von charging
  isCharging = (charging == 1);


  //Serial.println(Voltage);
  //Serial.println(akkustand);
  //Serial.println(charging);
}


//Display anzeigen 
//WLANsymbol
void drawWLANSymbol() {
  int centerX = tft.width() / 2;
  int topY = 10; // Abstand vom oberen Rand des Bildschirms für das WLANsymbol
  int radius = 5;  // radius kreis
  

  // Zeichne das größere Ladesymbol nur, wenn isCharging true ist
  if (isWifiConnected) {
      tft.fillCircle(centerX - radius /2,topY, radius, TFT_SILVER);
      tft.drawCircle(centerX - radius / 2, topY, radius, TFT_SILVER);

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
        tft.drawRect(centerX - rectWidth / 2, topY, rectWidth, rectHeight, TFT_SILVER);
        tft.fillRect(centerX - rectWidth / 2, topY, rectWidth, rectHeight, TFT_SILVER);
        // Zeichne das kleinere Rechteck rechts neben dem größeren Rechteck
        tft.drawRect(centerX - rectWidth / 2 + rectWidth + spaceBetween, topY + (rectHeight - smallRectHeight) / 2, smallRectWidth, smallRectHeight, TFT_SILVER);
        tft.fillRect(centerX - rectWidth / 2 + rectWidth + spaceBetween, topY + (rectHeight - smallRectHeight) / 2, smallRectWidth, smallRectHeight, TFT_SILVER);
    }
}

void zeichneDreieck(int x, int y, int width, int height, uint16_t color, bool spitzeNachUnten) {
  int x0, y0, x1, y1, x2, y2;

  if (!spitzeNachUnten) {
    // Spitze nach oben
    x0 = x + width / 2 + 40;    // Spitze in der Mitte der Breite, 20 Pixel nach rechts verschoben
    y0 = y;
    x1 = x;                    // Linke Basis
    y1 = y + height;
    x2 = x + width;            // Rechte Basis
    y2 = y + height;
  } else {
    // Spitze nach unten
    x0 = x + width / 2 - 40;    // Spitze in der Mitte der Breite, 20 Pixel nach links verschoben (für das zweite Dreieck)
    y0 = y + height;
    x1 = x;                    // Linke Basis
    y1 = y;
    x2 = x + width;            // Rechte Basis
    y2 = y;
  }

  // Fülle das Dreieck
  tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void symbolladen() {
  tft.fillScreen(TFT_BLACK); // Setze den Hintergrund des gesamten Bildschirms auf Schwarz

  // Definiere die Größe und den Abstand der Dreiecke
  int breite = 40;
  int hoehe = 60;
  int abstand = 0; // Optionaler Abstand zwischen den Dreiecken

  // Position des ersten Dreiecks (Spitze nach oben)
  int x1 = (tft.width() - 2 * breite - abstand) / 2; // Zentriert beide Dreiecke auf dem Bildschirm
  int y = (tft.height() - hoehe) / 2; // Vertikale Positionierung für beide Dreiecke

  // Zeichne das erste Dreieck (Spitze nach oben)
  zeichneDreieck(x1, y -25, breite, hoehe, TFT_YELLOW, false); 

  // Position des zweiten Dreiecks (Spitze nach unten)
  int x2 = x1 + breite + abstand; // Direkt neben dem ersten Dreieck

  // Zeichne das zweite Dreieck (Spitze nach unten)
  zeichneDreieck(x2, y + 25, breite, hoehe, TFT_YELLOW, true); 
}

//Akkuanzeige
void drawBatteryLevel(float batteryLevel) {
  tft.setTextSize(4);
  // Lösche zuerst den gesamten Bildschirm
  tft.fillRect(0, 0, tft.width(), tft.height(), TFT_BLACK);

  // Berechne die Füllhöhe basierend auf dem Akkustand
  int fillHeight = tft.height() * batteryLevel;

  // Wähle die Farbe basierend auf dem Akkustand
  uint16_t backgroundColor;
  if (batteryLevel > 0.51) {
    backgroundColor = TFT_GREEN;  // Grün für 100-50%
  } else if (batteryLevel > 0.21) {
    backgroundColor = TFT_YELLOW; // Gelb für 49-20%
  } else {
    backgroundColor = TFT_RED;    // Rot für 19-0%
  }

  // Fülle den unteren Bereich des Bildschirms mit der gewählten Farbe
  tft.fillRect(0, tft.height() - fillHeight, tft.width(), fillHeight, backgroundColor);

  // Definiere den Bereich für den Prozenttext
  int textX = tft.width() / 2;
  int textY = tft.height() / 2;

  // Bereite den Akkustand als Text vor
  char batteryStr[10];
  sprintf(batteryStr, "%d%%", (int)(batteryLevel * 100));

  // Setze die Textfarbe und zeichne den Akkustand
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Weißer Text mit schwarzem Hintergrund
  tft.drawString(batteryStr, textX, textY, 2); // Verwende die richtige Schriftgröße
}

//Mood Glücklich für akku von 100-50%
void drawSmiley100_50() {
  tft.fillScreen(TFT_BLACK); // Lösche den Bildschirm

  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;

  int eyeOffsetX = 40; // Verringerte horizontale Position der Augen für weniger Abstand
  int eyeOffsetY = 60; // Vertikale Position der Augen
  int eyeWidth = 40; // Breite der "^"-Augen
  int eyeHeight = 50; // Höhe der "^"-Augen
  int lineWidth = 3; // Breite der Linien

  // Funktion zum Zeichnen dickerer Linien
  auto drawThickLine = [&](int x0, int y0, int x1, int y1, uint16_t color, int width) {
    for (int i = -width / 2; i <= width / 2; ++i) {
      tft.drawLine(x0 + i, y0, x1 + i, y1, color);
    }
  };
    // Zeichne "^"-Augen mit dickeren Linien
    // Linkes Auge
    drawThickLine(centerX - eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY + eyeHeight,
                  centerX - eyeOffsetX, centerY - eyeOffsetY, TFT_WHITE, lineWidth);
    drawThickLine(centerX - eyeOffsetX, centerY - eyeOffsetY,
                  centerX - eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY + eyeHeight, TFT_WHITE, lineWidth);

    // Rechtes Auge
    drawThickLine(centerX + eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY + eyeHeight,
                  centerX + eyeOffsetX, centerY - eyeOffsetY, TFT_WHITE, lineWidth);
    drawThickLine(centerX + eyeOffsetX, centerY - eyeOffsetY,
                  centerX + eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY + eyeHeight, TFT_WHITE, lineWidth);

    // Funktion zum Zeichnen dickerer Bögen ohne Füllung
    auto drawThickArc = [&](int x, int y, int radius, uint32_t startAngle, uint32_t endAngle, uint32_t color, int thickness) {
      int outerRadius = radius; // Äußerer Radius des Bogens
      int innerRadius = radius - thickness; // Innenradius des Bogens für die Dicke

      if (innerRadius < 0) innerRadius = 0; // Stellen Sie sicher, dass der Innenradius nicht negativ ist

      // Zeichnen des Bogens. Beachten Sie, dass für bg_color hier die gleiche Farbe wie fg_color verwendet wird, um keinen Füllungseffekt zu haben.
      tft.drawArc(x, y, outerRadius, innerRadius, startAngle, endAngle, color, color, true); // Das letzte 'true' aktiviert das Glätten des Bogens, falls gewünscht.
    };

    // Zeichne Mund in Form von zwei Halbkreisen, die oben geöffnet sind
    int mouthCenterX = centerX; // Zentrum des Mundes (horizontal)
    int mouthCenterY = centerY + 25; // Zentrum des Mundes (vertikal, etwas unterhalb der Augen)
    int mouthRadius = 25; // Radius der Halbkreise
    int mouthLineWidth = 1; // Breite der Mundlinien
    int gab = 12;

    // Linker Halbkreis, oben geöffnet
    drawThickArc(mouthCenterX - mouthRadius / 2 -gab	, mouthCenterY, mouthRadius, 270, 90, TFT_WHITE, mouthLineWidth);
    // Rechter Halbkreis, oben geöffnet
    drawThickArc(mouthCenterX + mouthRadius / 2 + gab, mouthCenterY, mouthRadius, 270, 90, TFT_WHITE, mouthLineWidth);
}
//Mood Monoton für akku 50-20%
void drawSmiley50_20() {
  tft.fillScreen(TFT_BLACK); // Lösche den Bildschirm

  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;

  int eyeOffsetX = 40; // Verringerte horizontale Position der Augen für weniger Abstand
  int eyeOffsetY = 60; // Vertikale Position der Augen
  int eyeWidth = 40; // Breite der "^"-Augen
  int eyeHeight = 50; // Höhe der "^"-Augen
  int lineWidth = 3; // Breite der Linien

  // Funktion zum Zeichnen dickerer Linien
  auto drawThickLine = [&](int x0, int y0, int x1, int y1, uint16_t color, int width) {
    for (int i = -width / 2; i <= width / 2; ++i) {
      tft.drawLine(x0 + i, y0, x1 + i, y1, color);
    }
  };
    // Zeichne "^"-Augen mit dickeren Linien
    // Linkes Auge
    drawThickLine(centerX - eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY + eyeHeight,
                  centerX - eyeOffsetX, centerY - eyeOffsetY, TFT_WHITE, lineWidth);
    drawThickLine(centerX - eyeOffsetX, centerY - eyeOffsetY,
                  centerX - eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY + eyeHeight, TFT_WHITE, lineWidth);

    // Rechtes Auge
    drawThickLine(centerX + eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY + eyeHeight,
                  centerX + eyeOffsetX, centerY - eyeOffsetY, TFT_WHITE, lineWidth);
    drawThickLine(centerX + eyeOffsetX, centerY - eyeOffsetY,
                  centerX + eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY + eyeHeight, TFT_WHITE, lineWidth);

  

    // Zeichne Mund in Form von zwei Halbkreisen, die oben geöffnet sind
    int mouthCenterX = centerX; // Zentrum des Mundes (horizontal)
    int mouthCenterY = centerY + 25; // Zentrum des Mundes (vertikal, etwas unterhalb der Augen)
    int mouthLineWidth = 20; // Breite der Mundlinien
        // Mund als waagerechter Strich
    int mouthWidth = 70; // Die gewünschte Breite des Mundes

    // Anfangspunkt des Mundes (links)
    int mouthStartX = mouthCenterX - mouthWidth / 2;
    int mouthY = mouthCenterY; // Die y-Koordinate bleibt für Anfangs- und Endpunkt gleich

    // Endpunkt des Mundes (rechts)
    int mouthEndX = mouthCenterX + mouthWidth / 2;

    // Zeichne den Mund als waagerechten Strich
    drawThickLine(mouthStartX, mouthY, mouthEndX, mouthY, TFT_WHITE, mouthLineWidth);
}
//Mood Traurig für akku 20-0%
void drawSmiley20_0() {
  tft.fillScreen(TFT_BLACK); // Lösche den Bildschirm

  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;

  int eyeOffsetX = 10; // Verringerte horizontale Position der Augen für weniger Abstand
  int eyeOffsetY = 25; // Vertikale Position der Augen
  int eyeWidth = 80; // Höhe
  int eyeHeight = 50; // Breite
  int lineWidth = 3; // Breite der Linien

  // Funktion zum Zeichnen dickerer Linien
  auto drawThickLine = [&](int x0, int y0, int x1, int y1, uint16_t color, int width) {
    for (int i = -width / 2; i <= width / 2; ++i) {
      tft.drawLine(x0 + i, y0, x1 + i, y1, color);
    }
  };
     /// Zeichne "><"-Augen mit dickeren Linien, Spitzen zeigen zur Mitte
// Linkes Auge
drawThickLine(centerX - eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des linken Auges
              centerX - eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY - eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt oben links
drawThickLine(centerX - eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des linken Auges
              centerX - eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY + eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt unten links

// Rechtes Auge
drawThickLine(centerX + eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des rechten Auges
              centerX + eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY - eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt oben rechts
drawThickLine(centerX + eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des rechten Auges
              centerX + eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY + eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt unten rechts



    // Funktion zum Zeichnen dickerer Bögen ohne Füllung
    auto drawThickArc = [&](int x, int y, int radius, uint32_t startAngle, uint32_t endAngle, uint32_t color, int thickness) {
      int outerRadius = radius; // Äußerer Radius des Bogens
      int innerRadius = radius - thickness; // Innenradius des Bogens für die Dicke

      if (innerRadius < 0) innerRadius = 0; // Stellen Sie sicher, dass der Innenradius nicht negativ ist

      // Zeichnen des Bogens. Beachten Sie, dass für bg_color hier die gleiche Farbe wie fg_color verwendet wird, um keinen Füllungseffekt zu haben.
      tft.drawArc(x, y, outerRadius, innerRadius, startAngle, endAngle, color, color, true); // Das letzte 'true' aktiviert das Glätten des Bogens, falls gewünscht.
    };

    // Zeichne Mund in Form von zwei Halbkreisen, die oben geöffnet sind
    int mouthCenterX = centerX; // Zentrum des Mundes (horizontal)
    int mouthCenterY = centerY + 50; // Zentrum des Mundes (vertikal, etwas unterhalb der Augen)
    int mouthRadius = 30; // Radius der Halbkreise
    int mouthLineWidth = 1; // Breite der Mundlinien
    int gab = 12;

    // Mund
    drawThickArc(mouthCenterX - mouthRadius / 2 + gab	, mouthCenterY, mouthRadius, 90, 270, TFT_WHITE, mouthLineWidth);
}
//Mood Schalfen beim laden 
void drawSmileyladen() {
  tft.fillScreen(TFT_BLACK); // Lösche den Bildschirm

   int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;

  int eyeOffsetX = 10; // Verringerte horizontale Position der Augen für weniger Abstand
  int eyeOffsetY = 25; // Vertikale Position der Augen
  int eyeWidth = 80; // Höhe
  int eyeHeight = 50; // Breite
  int lineWidth = 3; // Breite der Linien

  // Funktion zum Zeichnen dickerer Linien
  auto drawThickLine = [&](int x0, int y0, int x1, int y1, uint16_t color, int width) {
    for (int i = -width / 2; i <= width / 2; ++i) {
      tft.drawLine(x0 + i, y0, x1 + i, y1, color);
    }
  };
     /// Zeichne "><"-Augen mit dickeren Linien, Spitzen zeigen zur Mitte
// Linkes Auge
drawThickLine(centerX - eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des linken Auges
              centerX - eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY - eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt oben links
drawThickLine(centerX - eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des linken Auges
              centerX - eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY + eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt unten links

// Rechtes Auge
drawThickLine(centerX + eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des rechten Auges
              centerX + eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY - eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt oben rechts
drawThickLine(centerX + eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des rechten Auges
              centerX + eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY + eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt unten rechts


    // Funktion zum Zeichnen dickerer Bögen ohne Füllung
    auto drawThickArc = [&](int x, int y, int radius, uint32_t startAngle, uint32_t endAngle, uint32_t color, int thickness) {
      int outerRadius = radius; // Äußerer Radius des Bogens
      int innerRadius = radius - thickness; // Innenradius des Bogens für die Dicke

      if (innerRadius < 0) innerRadius = 0; // Stellen Sie sicher, dass der Innenradius nicht negativ ist

      // Zeichnen des Bogens. Beachten Sie, dass für bg_color hier die gleiche Farbe wie fg_color verwendet wird, um keinen Füllungseffekt zu haben.
      tft.drawArc(x, y, outerRadius, innerRadius, startAngle, endAngle, color, color, true); // Das letzte 'true' aktiviert das Glätten des Bogens, falls gewünscht.
    };

    // Zeichne Mund in Form von zwei Halbkreisen, die oben geöffnet sind
    int mouthCenterX = centerX; // Zentrum des Mundes (horizontal)
    int mouthCenterY = centerY + 25; // Zentrum des Mundes (vertikal, etwas unterhalb der Augen)
    int mouthRadius = 20; // Radius der Halbkreise
    int mouthLineWidth = 1; // Breite der Mundlinien
    int gab = 10;

    // Linker Halbkreis, oben geöffnet
    drawThickArc(mouthCenterX - mouthRadius / 2 -gab	, mouthCenterY, mouthRadius, 270, 90, TFT_WHITE, mouthLineWidth);
    // Rechter Halbkreis, oben geöffnet
    drawThickArc(mouthCenterX + mouthRadius / 2 + gab, mouthCenterY, mouthRadius, 270, 90, TFT_WHITE, mouthLineWidth);

    // Definiere die Position und Größe des "Z"
    int zWidth = 5; // Die Breite des "Z"
    int zHeight = 10; // Die Höhe des "Z"
    int zLineWidth = 3; // Die Linienbreite des "Z"
    uint16_t zColor = TFT_BLUE; // Die Farbe des "Z"

    // Die obere linke Ecke des "Z" positionieren
    int zStartX = mouthCenterX - zWidth + 20 / 2 ;
    int zStartY = mouthCenterY - 20; // Positioniere das "Z" oberhalb des Mundes

    // Zeichne die obere horizontale Linie des "Z"
    drawThickLine(zStartX + 50, zStartY, zStartX + zWidth + 50 , zStartY, zColor, zLineWidth);
    drawThickLine(zStartX + 62, zStartY - 12, zStartX + zWidth + 62, zStartY - 12, zColor, zLineWidth);
    drawThickLine(zStartX + 74, zStartY - 29, zStartX + zWidth + 79 , zStartY - 29, zColor, zLineWidth);
    

    // Zeichne die diagonale Linie des "Z"
    drawThickLine(zStartX + zWidth + 50, zStartY, zStartX + 50 , zStartY + zHeight, zColor, zLineWidth);
    drawThickLine(zStartX + zWidth + 62, zStartY - 12, zStartX + 62 , zStartY + zHeight - 12, zColor, zLineWidth);
    drawThickLine(zStartX + zWidth + 79, zStartY - 29, zStartX + 74 , zStartY + zHeight - 24, zColor, zLineWidth);
   

    // Zeichne die untere horizontale Linie des "Z"
    drawThickLine(zStartX + 50, zStartY + zHeight, zStartX + zWidth + 50, zStartY + zHeight, zColor, zLineWidth);
    drawThickLine(zStartX + 62, zStartY + zHeight - 12, zStartX + zWidth + 62, zStartY + zHeight - 12, zColor, zLineWidth);
    drawThickLine(zStartX + 74, zStartY + zHeight - 24, zStartX + zWidth + 79, zStartY + zHeight - 24, zColor, zLineWidth);
    
    }
//Akkuanzeige anhand der Moods
void drawMoodBasedOnBatteryLevel(float batteryLevel) {
    // Lösche den Bildschirm vor dem Zeichnen eines neuen Mood
    tft.fillScreen(TFT_BLACK);

    if (batteryLevel > 0.51) { // Akkustand über 50%
        drawSmiley100_50(); // Zeichne glückliches Smiley
    } else if (batteryLevel > 0.21) { // Akkustand zwischen 20% und 50%
        drawSmiley50_20(); // Zeichne monotonen Smiley
    } else { // Akkustand unter 20%
        drawSmiley20_0(); // Zeichne trauriges Smiley
    }
}

void drawThinArc(int centerX, int centerY, int radius, int startAngle, int endAngle, uint16_t color) {
    for (int angle = startAngle; angle <= endAngle; angle++) {
        float rad = angle * DEG_TO_RAD; // Umrechnen von Grad in Radiant
        int x = centerX + radius * cos(rad);
        int y = centerY + radius * sin(rad);
        tft.drawPixel(x, y, color); // Setze einen einzelnen Pixel an der berechneten Position
    }
}

//Nachdenken
void drawSmileyThinking() {
    tft.fillScreen(TFT_BLACK); // Lösche den Bildschirm

    int centerX = tft.width() / 2;
    int centerY = tft.height() / 2;

    int eyeOffsetX = 10; // Horizontale Position der Augen
    int eyeOffsetY = 25; // Vertikale Position der Augen
    int eyeWidth = 80; // Breite der Augen
    int eyeHeight = 50; // Höhe der Augen
    int lineWidth = 2; // Breite der Linien

    // Funktion zum Zeichnen dickerer Linien
    auto drawThickLine = [&](int x0, int y0, int x1, int y1, uint16_t color, int width) {
        for (int i = -width / 2; i <= width / 2; ++i) {
            tft.drawLine(x0 + i, y0, x1 + i, y1, color);
        }
    };

    // Linkes Auge
drawThickLine(centerX - eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des linken Auges
              centerX - eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY - eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt oben links
drawThickLine(centerX - eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des linken Auges
              centerX - eyeOffsetX - eyeWidth / 2, centerY - eyeOffsetY + eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt unten links

// Rechtes Auge
drawThickLine(centerX + eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des rechten Auges
              centerX + eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY - eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt oben rechts
drawThickLine(centerX + eyeOffsetX, centerY - eyeOffsetY, // Startpunkt in der Mitte des rechten Auges
              centerX + eyeOffsetX + eyeWidth / 2, centerY - eyeOffsetY + eyeHeight / 2, TFT_WHITE, lineWidth); // Endpunkt unten rechts

    // Zeichne einen nachdenklichen Mund
    int mouthWidth = 65;
    int mouthStartX = centerX - mouthWidth / 2;
    int mouthStartY = centerY + 15; // Etwas niedriger als die Augen

    // Mund als leicht geneigte Linie für ein nachdenkliches Aussehen
    drawThickLine(mouthStartX, mouthStartY,mouthStartX + mouthWidth, mouthStartY + 10, TFT_WHITE, lineWidth + 5);

    // Zeichne ein Fragezeichen neben das rechte Auge
    int qMarkStartX = centerX + eyeOffsetX + eyeWidth - 20; // Start etwas rechts vom rechten Auge
    int qMarkStartY = centerY - eyeOffsetY - eyeHeight + 60; // Start etwas oberhalb des rechten Auges
    int qMarkSize = 15; // Radius des Bogens für das Fragezeichen

    // Oberer Bogen des Fragezeichens, horizontal orientiert
    drawThinArc(qMarkStartX, qMarkStartY, qMarkSize, 270, 450, TFT_WHITE);

    // Verschiebe die Startposition der vertikalen Linie und des Punktes nach unten
    int lineAndDotStartY = qMarkStartY + qMarkSize; // Verschiebe etwas nach unten
    int lineAndDotsStartX = qMarkStartX + qMarkSize - 30; // Verschiebe x achse

    // Vertikale Linie des Fragezeichens, etwas nach unten verschoben
    drawThickLine(lineAndDotsStartX + qMarkSize, lineAndDotStartY, lineAndDotsStartX + qMarkSize, lineAndDotStartY + qMarkSize -5, TFT_WHITE, 1);

    // Punkt des Fragezeichens, ebenfalls nach unten verschoben
    tft.fillCircle(lineAndDotsStartX + qMarkSize, lineAndDotStartY + qMarkSize, 1, TFT_WHITE);
}

// Funktion zum Zeichnen eines gefüllten Segments
void drawFilledSegment(int centerX, int centerY, int radius, float startAngle, float endAngle, uint32_t color) {
  float segmentSize = 5.0; // Segmentgröße in Grad

  for (float angle = startAngle; angle < endAngle; angle += segmentSize) {
    float angleRad = angle * DEG_TO_RAD; // Umrechnung in Radiant
    float nextAngleRad = (angle + segmentSize) * DEG_TO_RAD;

    // Berechne die Koordinaten der Eckpunkte des Segments
    int x1 = centerX + radius * cos(angleRad);
    int y1 = centerY + radius * sin(angleRad);
    int x2 = centerX + radius * cos(nextAngleRad);
    int y2 = centerY + radius * sin(nextAngleRad);

        tft.fillTriangle(centerX, centerY, x1, y1, x2, y2, color);
  }
}

// Funktion zur Aktualisierung der Timer-Anzeige
void updateTimerDisplay() {
  tft.setTextSize(4);
  
  // Berechne den Winkel basierend auf der verstrichenen Zeit
  float angle = 360.0 * (float)elapsedSeconds / 60.0; // 60.0 für eine volle Minute

  tft.fillScreen(TFT_BLACK); // Lösche den Bildschirm

  // Zeichne den gefüllten Sektor
  int centerX = tft.width() / 2;
  int centerY = tft.height() / 2;
  int radius = min(tft.width(), tft.height()) / 2;
  drawFilledSegment(centerX, centerY, radius, 0, angle, TFT_CYAN);

  // Bereite die Zeit als Zeichenkette vor und zeichne sie
  int minutes = elapsedSeconds / 60;
  int seconds = elapsedSeconds % 60;
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(timeStr, centerX, centerY);
}

// Timer-Funktion
void time() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    elapsedSeconds++;  // Inkrementiere die verstrichene Zeit jede Sekunde
    //updateTimerDisplay();  // Aktualisiere die Anzeige

    if (elapsedSeconds >= 60) {
      elapsedSeconds = 0;  // Setze die Sekunden zurück, wenn eine Minute erreicht ist
      // Optional: Code zum Inkrementieren der Minuten und Aktualisieren der Anzeige
    }
  }
}


void fahren(){
  static unsigned long lastChangeMillis = 0;
    static int state = 0;
    static unsigned long timerPreviousMillis = 0;
    static unsigned long displayPreviousMillis = 0;
    const long timerInterval = 1000;
    const long displayInterval = 5000;
    unsigned long currentMillis = millis();

    // Timer-Update-Logik
    if (currentMillis - timerPreviousMillis >= timerInterval) {
        timerPreviousMillis = currentMillis;
        if (remainingSeconds > 0) {
            remainingSeconds--;
            if (state == 0) {
                updateTimerDisplay();
            }
        }
    }

    // Display-Update-Logik
    if (currentMillis - displayPreviousMillis >= displayInterval) {
        displayPreviousMillis = currentMillis;
        state = (state + 1) % 5; // Wechselt zwischen den Zuständen

        // Abhängig vom Zustand werden verschiedene Funktionen aufgerufen
        switch (state) {
            case 0:
                updateTimerDisplay();  // Aktualisiere und zeige den Timer
                break;
            case 1:
                drawMoodBasedOnBatteryLevel(akkustand);
                break;
            case 2:
                drawBatteryLevel(akkustand);  // Zeichne die Akkuanzeige
                drawChargingSymbol();
                break;
            case 3:
                drawWiFiDetails();  // Zeichne die WLAN-Details
                drawWLANSymbol();  // Zeichne das WLAN-Symbol
                break;
            // Hier könnten weitere Zustände behandelt werden
        }
    }

}

void laden(){
  static unsigned long lastChangeMillis = 0;
  static int state = 0;
  static unsigned long timerPreviousMillis = 0;
  static unsigned long displayPreviousMillis = 0;
  const long displayInterval = 5000;
  unsigned long currentMillis = millis();

  if (currentMillis - displayPreviousMillis >= displayInterval) {
        displayPreviousMillis = currentMillis;
        state = (state + 1) % 2; // Wechselt zwischen den Zuständen

        // Abhängig vom Zustand werden verschiedene Funktionen aufgerufen
        switch (state) {
            case 0:
                symbolladen();  // Aktualisiere und zeige den Timer
                break;
            case 1:
                drawSmileyladen();
                break;
        }
  }
}

void setup(void) {
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Serial.begin(9600);
  Wire.begin();

  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) {}  // Stoppe alles, wenn keine Verbindung zum MPU6050 hergestellt werden konnte

  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets();  // Kalibriere Gyro und Accelerometer
  Serial.println("Done!\n");

  tft.init();
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialisiere die Motoren mit der gewünschten Geschwindigkeit
  stepper1.setMaxSpeed(maxSpeed);
  stepper1.setSpeed(desiredSpeed);

  stepper2.setMaxSpeed(maxSpeed);
  stepper2.setSpeed(desiredSpeed);

  // sonstige eingaben 
  pinMode(Enable, OUTPUT);      //Enable
  digitalWrite(Enable,LOW);     //Enable
  pinMode(sensorPin, INPUT);    //Setze den Sensor-Pin als Eingang
  pinMode(wisch, OUTPUT);       //Setze das Wischmodula als Ausgang 
  pinMode(taster1, INPUT);      //Setzt Taster als eingang
  pinMode(taster2, INPUT);      //setzt Taster 2 als eingang 
  pinMode(BL,OUTPUT);           //BL als Ausgang
    pinMode(13,INPUT_PULLDOWN);

    
    // Initialisiere den Timer und zeige ihn an
    remainingSeconds = totalSeconds;
    updateTimerDisplay();

    digitalWrite(BL,HIGH); //Display Backlight anschalten
}

//Roboter machz eine 90° Drehung nach rechts
void turnRight()
{
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);
  
  float degree = 2100;
  float moveRev = degree * SteppDegree;
  stepper1.move(moveRev);
  stepper2.move(-moveRev);

    // Führe die Drehung aus und warte, bis sie abgeschlossen ist
  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}

//Roboter macht eine 90° Drehung nach links
void turnLeft()
{
  
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);
  
  float degree = 2100;
  float moveRev = degree * SteppDegree;
  stepper1.move(-moveRev);
  stepper2.move(moveRev);

    // Führe die Drehung aus und warte, bis sie abgeschlossen ist
  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
  
}

// Roboter macht eine 180° Drehung nach links 
void turnleft180()
{
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);
  
  float degree = 4200;
  float moveRev = degree * SteppDegree;
  stepper1.move(-moveRev);
  stepper2.move(moveRev);

    // Führe die Drehung aus und warte, bis sie abgeschlossen ist
  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}

//Roboter bewegt sich vorwärts
void moveForward() {
  stepper1.setSpeed(desiredSpeed);
  stepper2.setSpeed(desiredSpeed);
  stepper1.runSpeed();
  stepper2.runSpeed();
}

//Roboter fährt einen kleinen versatzt von 1000 schritte
void moveShortDistance(int steps) {
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);

  stepper1.move(steps);
  stepper2.move(steps);
  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}

void moveShortDistanceback(int steps) {
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);

  stepper1.move(-steps);
  stepper2.move(-steps);

  while (stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0) {
    stepper1.run();
    stepper2.run();
  }
}

//Motoren Stoppen
void stopMotors() {
  stepper1.stop(); // Stoppe Stepper 1
  stepper2.stop(); // Stoppe Stepper 2
  while (stepper1.isRunning() || stepper2.isRunning()) {
    // Warte, bis beide Motoren vollständig angehalten haben
    stepper1.run();
    stepper2.run();
  }
}

//comming home von oben rechts
void homeor() {
  int sensorValue = digitalRead(sensorPin);
    // if (isPreparingForHome== true) {
    //     // Überspringe den ersten Case, gehe direkt zu MOVING_FORWARD2_HOME
    //     homeState2 = MOVING_FORWARD2_HOME;
    //     isPreparingForHome = false; // Zurücksetzen des Flags
    // }
  switch(homeState2) {

    case TURNING_LEFT2_HOME:
    turnleft180();
    homeState2 = MOVING_FORWARD2_HOME;
    break;

    case MOVING_FORWARD2_HOME:
    moveForward();
    if (sensorValue == HIGH) {
      homeState2 = TURNING_LEFT2_HOME2;
    }
      break;

    case TURNING_LEFT2_HOME2:
    turnLeft();
    homeState2 = MOVING_FORWARD2_HOME2;
    break;

    case MOVING_FORWARD2_HOME2:
    moveForward();
    if (sensorValue == HIGH) {
      homeState2 = TURNING_LEFT2_HOME3;
    }
    break;
  
    case TURNING_LEFT2_HOME3:
      turnleft180();
      homeState2 = STOPPING_HOME2;
      break;

    case STOPPING_HOME2:
      stopMotors();
      // currentState = MOVING_FORWARD;
      break;

  }
}


//comming Home von unten rechts
void homeur() {
  int sensorValue = digitalRead(sensorPin); // Lese den Sensorwert

  switch (homeState) {


    case TURNING_LEFT_HOME:
      turnLeft(); // Führe eine Linksdrehung aus
      homeState = MOVING_FORWARD_HOME; // Wechsle den Zustand zu vorwärts bewegen
      break;

    case MOVING_FORWARD_HOME:
      moveForward(); // Bewege dich vorwärts
      if (sensorValue == HIGH) { // Wenn der Sensor 1 ausgibt
        homeState = TURNING_LEFT_HOME2;
      } 
      break;

    case TURNING_LEFT_HOME2:
    turnLeft();
    homeState = MOVING_FORWARD_HOME2;
    break;

    case MOVING_FORWARD_HOME2:
      moveForward(); // Bewege dich vorwärts
      if (sensorValue == HIGH) { // Wenn der Sensor 1 ausgibt
        homeState = TURNING_LEFT_HOME3;
      } 
      break;

    case TURNING_LEFT_HOME3:
    turnLeft();
    homeState = MOVING_FORWARD_HOME3;
    break;

    case MOVING_FORWARD_HOME3:
      moveForward(); // Bewege dich vorwärts
      if (sensorValue == HIGH) { // Wenn der Sensor 1 ausgibt
        homeState = TURNING_LEFT_HOME4;
      }
      break;

    case TURNING_LEFT_HOME4:
    turnleft180();
    homeState = STOPPING_HOME;
    break;


    case STOPPING_HOME:
      stopMotors(); // Stoppe die Motoren
      // Zurücksetzen oder weitere Aktionen nach dem Anhalten
      break;
  }
}


void loop() {
  time();
  fahren();

//Webserver Input
    static unsigned long previousMillis = 0; // Speichert den letzten Zeitpunkt, zu dem der Akkustand aktualisiert wurde
    const long interval = 100; // Aktualisierungsintervall in Millisekunden (1 Sekunde)

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        //   if (isCalibratingY) {
        //     yabmessung++; // Zähle hoch, wenn in Y-Kalibrierung
        //    }
        //   if (isCalibratingX) {
        //     xabmessung++; // Zähle hoch, wenn in X-Kalibrierung
        //   }

        
        
        
        
        previousMillis = currentMillis;

        getBattery();
        /*if (roboterInBetrieb) {
            // Verringere den Akkustand nur, wenn der Roboter in Betrieb ist
            akkustand = akkustand > 0 ? akkustand - 1 : 100; // Verringere den Akkustand um 1% jede Sekunde, setze zurück auf 100% bei 0
            Serial.println("Akkustand: " + String(akkustand) + "%");
        }*/
    }
    //server.handleClient();
    if (roboterInBetrieb) {
    unsigned long aktuelleZeit = millis();
    verstricheneZeit = (aktuelleZeit - startZeitpunkt) / 1000; // Berechnet die verstrichene Zeit in Sekunden
    }
    // ueberprueft, ob die Wartezeit nach dem Stopp-Befehl abgelaufen ist
  


  //Standardroute State 
  int sensorValue = digitalRead(sensorPin); // Lese den Sensorwert

  digitalWrite(wisch,HIGH); // wisch soll an gehen 
//  int taster2state = digitalRead(taster2); // Lesen Sie den Zustand der Taste
  // int lastButtonState = LOW; // Speichert den letzten Zustand des Buttons

  // if(turnLeftNext){
  //   richtung= "unten";
  //   }
  //   else{
  //   richtung="oben";
  //   }
  //Serial.println("switch currentState");
  switch (currentState) {
    
    // case CHECK_BUTTON:
    //     //Serial.println("CHECK_BUTTON");
    //     if (taster2state  == HIGH && lastButtonState == LOW) {
    //       //Serial.println("taster2if");
    // // Der Button wurde gerade gedrückt
    //     roboterInBetrieb = !roboterInBetrieb; // Wechseln Sie den Betriebszustand des Roboters
    //     }

    //     currentState = MOVING_FORWARD; // Wechseln Sie den Zustand zu MOVING_FORWARD
    //   lastButtonState = taster2state; 
    //   break;

    case MOVING_FORWARD:
      moveForward(); // Bewege dich vorwärts
      if (sensorValue == HIGH) { // Wenn der Sensor 1 ausgibt
      currentState = MOVEBACK;
      }
      break;

      case MOVEBACK:
      moveShortDistanceback(bist);
        if (turnLeftNext) {
          currentState = TURNING_LEFT; // Wechsle den Zustand zu Linksabbiegung
          turnLeftNext = false; // Setze zurück, damit das nächste Abbiegen wieder rechts ist
        } else {
          currentState = TURNING_RIGHT; // Wechsle den Zustand zu Rechtsabbiegung
        }
        break;


    case TURNING_RIGHT:
      turnRight(); // Führe eine Rechtsabbiegung aus
      if (digitalRead(sensorPin) == HIGH) {
        currentState = STOPPINGOR;
      } else {
        currentState = MOVING_SHORT_DISTANCER;
      }
      break;

    case MOVING_SHORT_DISTANCER:
      moveShortDistance(dist); // Bewege dich eine kurze Strecke vorwärts
      if (digitalRead(sensorPin) == HIGH) {
        currentState = STOPPINGOR;
      } else {
      currentState = TURNING_RIGHT2;
      }
      break;

    case TURNING_RIGHT2:
      turnRight(); // Führe die zweite Rechtsabbiegung aus
      currentState = MOVING_FORWARD;
      turnLeftNext = true; // Setze das Flag, damit beim nächsten Sensor HIGH links abgebogen wird
      break;

    case TURNING_LEFT:
      turnLeft(); // Führe eine Linksabbiegung aus
      // Überprüfe den Sensorwert direkt nach der Drehung
      if (digitalRead(sensorPin) == HIGH) {
        currentState = STOPPINGUR;
      } else {
        currentState = MOVING_SHORT_DISTANCEL; // Kehre zurück zum Zustand vorwärts bewegen
      }
      break;


    case MOVING_SHORT_DISTANCEL:
      moveShortDistance(dist);
      if (digitalRead(sensorPin) == HIGH){
        currentState = STOPPINGUR;
      } else {
      currentState = TURNING_LEFT2;
      }
      break;

    case TURNING_LEFT2:
      turnLeft();
      currentState = MOVING_FORWARD;
      break;


// Motor stoppt sobal er am ende der Tafel angekommen ist
    case STOPPINGUR:
      stopMotors();
      delay(2000);
      currentState = HOMEUR;
      break;


    case HOMEUR:
    homeur();
    break;

    case STOPPINGOR:
      stopMotors();
      delay(2000);
      currentState = HOMEOR;
      break;


    case HOMEOR:
    homeor();
    break;

  }

  //   case PREPARE_COMING_HOME_FROM_TOP:
  //       turnLeft(); // Drehe nach links
  //       currentState = HOMEOR;
  //       break;

  //   case PREPARE_COMING_HOME_FROM_BOTTOM:
  //       turnRight();
  //       currentState = HOMEOR;
  //       break;

  // }
  // if (digitalRead(taster2) == HIGH) {
  //   // Entscheidung basierend auf der aktuellen Richtung
  //   if (richtung == "unten") {
  //     isPreparingForHome = true;
  //     currentState =  STOPPINGOR;
  //   } else if (richtung == "oben") {
  //     isPreparingForHome = true;
  //     currentState =  STOPPINGOR;
  //   } else if (richtung == "rechts") {
  //     currentState = STOPPINGOR;
  // }

  
  if ((millis() - timer) > 10) {  // Daten alle 10ms ausgeben
    //Serial.print("Z : ");
    //Serial.println(mpu.getAngleZ());
    timer = millis();
    mpu.update();

    // Rotation des Displays anpassen basierend auf der Z-Achsen Neigung
    if (mpu.getAngleZ() < -45) { // Neigung nach rechts
      tft.setRotation(0); // Hier könntest du die Ausrichtung anpassen, z.B. keine Drehung
    } else if (mpu.getAngleZ() > 45) { // Neigung nach links
      tft.setRotation(2); // Hier könntest du die Ausrichtung anpassen, z.B. 180 Grad Drehung
    } else {
      // Wenn die Neigung innerhalb des Bereichs -45 bis 45 Grad liegt, könnte die Standardausrichtung beibehalten werden
      tft.setRotation(1); // Beibehalten der Standardausrichtung oder einer anderen festen Rotation
    }
  }


//Display Anzeige
  if(isCharging) {
    laden();
  }
  else {
    fahren();
  }
   }