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
const int Enable = 18;
// Pinbelegung Sonstige
const int taster1 = 34; //Taster stoppen 
const int taster2 = 35; // Taster Standardroute
const int sensorPin = 23; // Pin-Nummer des Sensors
const int wisch = 19; // Pin-Nummer wischmodul
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
enum State {MOVING_FORWARDcalib,
            TURNING_RIGHTcalib,MOVING_FORWARD2calib,
            MOVING_START_FORWARD, TURNING_RIGHT_START,MOVING_START_FORWARD2, TURNING_LEFT_START,
            MOVING_FORWARD,TURNING_RIGHT, TURNING_RIGHT2,
            MOVING_SHORT_DISTANCER, MOVING_SHORT_DISTANCEL,
            TURNING_LEFT, TURNING_LEFT2,
            STOPPINGUR, HOMEUR, STOPPINGOR, HOMEOR,
            PREPARE_COMING_HOME_FROM_BOTTOM,
            PREPARE_COMING_HOME_FROM_TOP,
            PREPARE_COMING_HOME_FROM_RIGHT,ACTIVE};
            
State currentState = MOVING_FORWARDcalib;
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
int akkustand = 0; // Beispiel fuer einen Akkustand
int aktuellerModus = 0; // Globale Variable fuer den aktuellen Modus
int fortschritt = 0;
int charging = 0;
int verstricheneZeit;
bool aufStart = true; // 
unsigned long startZeitpunkt = 0; 
unsigned long stoppZeitpunkt = 0; 
const unsigned long wartezeit = 5000; // 5 Sekunden Wartezeit in Millisekunden
bool zurueckkehren = false; // Neue Zustandsvariable fuer "Zurueckkehren"
int modus;

TFT_eSPI tft = TFT_eSPI();  // Initialisiere die Bibliothek für das TFT-Display

unsigned long previousMillis = 0;  // Speichert den letzten Zeitpunkt, zu dem das Display aktualisiert wurde
const long interval = 1000;  // Intervall für die Aktualisierung der Uhrzeit in Millisekunden (1000 ms = 1 Sekunde)

int startMinutes = 1;  // Startminuten für den Countdown
int startSeconds = 30;  // Startsekunden für den Countdown
int totalSeconds = startMinutes * 60 + startSeconds;  // Gesamtzeit des Countdowns in Sekunden
int remainingSeconds = totalSeconds;  // Verbleibende Zeit des Countdowns in Sekunden

bool isCharging = true; // Setzen Sie diese Variable entsprechend dem Ladezustand des Akkus
bool isWifiConnected = true; // setzten endprechen ob mit WLAn verbunden oder nicht 
bool kalib = true; // nur zum testen da um zu schauen ob die funtnkion get muss noch an alpay angebundne werden 

//Webserver Modi mit aktueller Modus Variable und Cases

void setModus() {
    if (!server.hasArg("modus")) {
        server.send(400, "text/plain", "Fehlender Modus Parameter");
        return;
        
    }

    aktuellerModus = server.arg("modus").toInt();
    roboterInBetrieb = true; // Nehmen wir an, der Roboter ist aktiv, sobald ein Modus gesetzt wird.
    startZeitpunkt = millis(); // Speichert den aktuellen Zeitpunkt
    // Hier geben wir in der seriellen Konsole den aktuellen Modus aus.
    switch (aktuellerModus) {
        case 0:
            Serial.println("aktueller Modus:Stopp");
            break;
        case 1:
            Serial.println("Aktueller Modus: Standardroute");
            break;
        case 2:
            Serial.println("Aktueller Modus: Oben Links");
            break;
        case 3:
            Serial.println("Aktueller Modus: Oben Rechts");
            break;
        case 4:
            Serial.println("Aktueller Modus: Unten Links");
            break;
        case 5:
            Serial.println("Aktueller Modus: Unten Rechts");
            break;
        case 6:
            Serial.println("Aktueller Modus: Kalibrieren");
            break;
        default:
            Serial.println("Unbekannter Modus");
            roboterInBetrieb = false; // Ungültiger Modus wurde gesetzt.
            break;
    }
    Serial.println("Modus gesetzt auf: " + String(aktuellerModus));
    server.send(200, "text/plain", "Modus gesetzt auf " + server.arg("modus"));
}

//Stopp Tafelwischen
void handleStopp() {
    if (roboterInBetrieb || zurueckkehren) { // Überprüfung, ob der Roboter in Betrieb ist oder zurückkehren soll
        roboterInBetrieb = false; // Der Roboter wird nicht mehr in Betrieb sein.
        verstricheneZeit = 0; // Reset der verstrichenen Zeit beim Stopp
        zurueckkehren = false; // Zurückkehren-Status zurücksetzen
        aktuellerModus = 0; // Zurück zum Standby-Modus setzen.
        Serial.println("Der Roboter wurde gestoppt und ist im Standby-Modus.");
        Serial.println("aktueller Modus: Stopp");
        server.send(200, "text/plain", "Roboter gestoppt. Aktueller Modus: Stopp.");
    } else {
        server.send(200, "text/plain", "Roboter ist bereits im Standby-Modus.");
    }
}
// Start Webserver
void handleRoot() {

    char html[] = R"rawliteral(
<html>
<head>
    <title>Tafelwischroboter Steuerung</title>
    <style>
     body {
        background-image: url('hintergrundbild.jpg');
        background-repeat: no-repeat;
        background-position: center;
        background-attachment: fixed;
        background-size: cover;
        }
        .button { background-color: #4CAF50; border: none; color: white; width:295px;height:50px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }
        .tafel { width: 600px; height: 400px; border: 1px solid black; display: flex; flex-wrap: wrap; }
        .tafel button { width: 50%; height: 50%; }
        .battery { width: 100px; height: 20px; border: 1px solid black; position: relative; display: inline-block; vertical-align: middle; }
        .battery-level { height: 100%; background-color: green; width: 0%; } /* Startet mit 0% Breite */
        .battery::after { content: ''; position: absolute; top: 4px; right: -6px; width: 5px; height: 12px; border: 1px solid black; background-color: black; }
        .status-box { border: 1px solid black;width: 580px; height: 20px;text-align: center;padding:10px;  }
        h1 {
    font-family: "Arial", serif;
}
        h10 {
    font-family: "Arial", serif;
}

    </style>
</head>
<body>
    <h1>SpudBoard Steuerung</h1>
    <div id="buttons">
        <button class='button' onclick="sendCommand('1')">Standardroute</button>
        <button class='button' onclick="sendCommand('6')">Kalibrieren</button>
        <div class='tafel'>
            <button onclick="sendCommand('2')">Viertel 1</button>
            <button onclick="sendCommand('3')">Viertel 2</button>
            <button onclick="sendCommand('4')">Viertel 3</button>
            <button onclick="sendCommand('5')">Viertel 4</button>
        </div>
        <button class='button' id="stopButton" onclick="sendCommand('stop')">Stopp</button>
    </div>
    <div id="infoBlock" style="display:none; font-size: 24px;">
        <p id="wischFortschritt"></p>
        <p id="wischZeit"></p>
    </div>
    <div class='battery'><div class='battery-level' id="batteryLevel"></div></div>
    <span id="batteryText">Akku: --%</span>
    <div class='status-box' id="robotStatus">Status: Standby</div>
    <div id="infoBlock" style="display:none; font-size: 24px;"> 
    <p id="wischFortschritt"></p>
    <p id="wischZeit"></p>
    </div>
    <h1>   </h1>
    <h10>Alpay Yalili</h10>
    <h10>Celine Ernst</h10>
    <h10>Tobias Kerschensteiner</h10>
    <script>
    function sendCommand(modus) {
    const url = modus === 'stop' ? '/stopp' : '/setModus?modus=' + modus;
    fetch(url)
        .then(response => response.text())
        .then(() => {
            updateUI(); // Stellen Sie sicher, dass updateUI den aktuellen Status korrekt reflektiert
        });
        }
    function updateUI() {
    fetch('/getStatus')
        .then(response => response.json())
        .then(data => {
            // Akkustandanzeige aktualisieren
            const batteryLevelDiv = document.getElementById('batteryLevel');
            const batteryText = document.getElementById('batteryText');
            batteryLevelDiv.style.width = data.akkustand + '%';
            batteryText.innerText = 'Akku: ' + data.akkustand + '%';

            document.getElementById("robotStatus").innerText = "Status: " + data.statusText;
            const stopButton = document.getElementById('stopButton');
            const modeButtons = document.querySelectorAll('.tafel .button, .tafel ,.button:not(#stopButton)');
            // Anzeigen der verstrichenen Zeit
            document.getElementById("wischZeit").innerText = "Verstrichene Zeit: " + data.zeit + " Sekunden";
            // Anzeigen des aktuellen Modus
            document.getElementById("robotStatus").innerText = "Status: " + data.modusName;
            // Anzeigen des Fortschritts
            document.getElementById("wischFortschritt").innerText = "Fortschritt: " + data.fortschritt + "%";
            const infoBlock = document.getElementById("infoBlock");
            if(data.roboterInBetrieb) {
                infoBlock.style.display = 'block'; // Zeige den Info-Block, wenn der Roboter aktiv ist
            } else {
                infoBlock.style.display = 'none'; // Verstecke den Info-Block, wenn der Roboter nicht aktiv ist
            }
            if (data.roboterInBetrieb) {
                // Roboter ist in Betrieb - zeige nur den "Stopp"-Knopf
                stopButton.style.display = '';
                modeButtons.forEach(button => button.style.display = 'none');
            } else {
                // Roboter ist nicht in Betrieb - zeige Modus-Knöpfe und verstecke "Stopp"-Knopf
                stopButton.style.display = 'none';
                modeButtons.forEach(button => button.style.display = '');
            }
        });
}

    function sendCommand(modus) {
        const url = modus === 'stop' ? '/stopp' : '/setModus?modus=' + modus;
        fetch(url)
            .then(response => response.text())
            .then(() => {
                updateUI();
                const stopButton = document.getElementById('stopButton');
                const modeButtons = document.querySelectorAll('.tafel .button');
                
                if (modus === 'stop') {
                    stopButton.style.display = 'none'; // Verstecke den Stopp-Knopf
                    modeButtons.forEach(button => button.style.display = ''); // Zeige die Modus-Knöpfe an
                } else {
                    stopButton.style.display = ''; // Zeige den Stopp-Knopf
                    modeButtons.forEach(button => button.style.display = 'none'); // Verstecke die Modus-Knöpfe
                }
            });
    }
    document.addEventListener('DOMContentLoaded', function() {
        updateUI(); // Initialisiere die UI beim Laden der Seite
        setInterval(updateUI, 1000); // Aktualisiere die UI alle 1 Sekunden
});
    </script>
</body>
</html>

)rawliteral";
    server.send(200, "text/html", html);
}
//Kommunikation Webserver -> Esp32
void getStatus() {
    DynamicJsonDocument doc(1024);
    doc["roboterInBetrieb"] = roboterInBetrieb;
    doc["akkustand"] = akkustand;
    doc["aktuellerModus"] = aktuellerModus;
    doc["aufStart"] = aufStart;
    doc["fortschritt"] = fortschritt;
    doc["zeit"] = verstricheneZeit;
    doc["zurueckkehren"] = zurueckkehren;

    String statusText = "Standby"; // Standardstatus
    if (roboterInBetrieb) {
        statusText = "aktiv"; // Setzen Sie den Status auf "aktiv", wenn der Roboter in Betrieb ist
    } else if (zurueckkehren) {
        statusText = "zurückkehrend"; // Ein weiterer möglicher Status
    }
    // Hier fügen wir den Status-Text zum JSON-Dokument hinzu
    doc["statusText"] = statusText;
    //zusätzliche Informationen für die Anzeige
        String modusName;
    switch (aktuellerModus) {
        case 0: modusName = "Inaktiv"; break;
        case 1: modusName = "Aktiv: Fährt Standardroute"; break;
        case 2: modusName = "Aktiv: Fährt Viertel Oben Links";break;
        case 3: modusName = "Aktiv: Fährt Viertel Oben Rechts";break;
        case 4: modusName = "Aktiv: Fährt Viertel Unten Links";break;
        case 5: modusName = "Aktiv: Fährt Viertel Unten Rechts";break;
        case 6: modusName = "Aktiv: Kalibrierung";break;
        default: modusName = "Unbekannt"; break;
    }
    doc["modusName"] = modusName;

    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

void getBattery()
{
  int Voltage = analogRead(34);

  akkustand = map(Voltage,2950,4095,0,120);

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

  //webserver Setup 
      Serial.begin(9600);

      pinMode(13,INPUT_PULLDOWN);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi verbunden.");
    Serial.println("IP-Adresse: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/setModus", setModus);
    server.on("/stopp", handleStopp);
    server.on("/stopp", []() {
    if (roboterInBetrieb || zurueckkehren) {
        roboterInBetrieb = false;
        zurueckkehren = false; // Setzen Sie die entsprechenden Zustände
        aufStart = true; // Erlauben Sie neue Befehle
        stoppZeitpunkt = millis(); // Reset der Zeit, falls benötigt
        server.send(200, "text/plain", "Roboter gestoppt und bereit für neue Befehle.");
    } else {
        server.send(200, "text/plain", "Roboter ist nicht in Betrieb.");
    }
    if (richtung == "unten") {
        currentState = PREPARE_COMING_HOME_FROM_BOTTOM;
        isPreparingForHome = true;
    } else if (richtung == "oben") {
        currentState = PREPARE_COMING_HOME_FROM_TOP;
        isPreparingForHome = true;
    } else if (richtung == "rechts") {
        currentState = PREPARE_COMING_HOME_FROM_RIGHT;
    }
    server.send(200, "text/plain", "Roboter gestoppt. Kehrt nach Hause zurück, falls nötig.");
});
    currentState = MOVING_FORWARDcalib; // Starte mit Kalibrierung
    server.on("/getStatus", getStatus);
    server.begin();
    Serial.println("Webserver gestartet");

    
    // Initialisiere den Timer und zeige ihn an
    remainingSeconds = totalSeconds;
    updateTimerDisplay();
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
  stepper1.move(steps);
  stepper2.move(steps);
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
    if (isPreparingForHome) {
        // Überspringe den ersten Case, gehe direkt zu MOVING_FORWARD2_HOME
        homeState2 = MOVING_FORWARD2_HOME;
        isPreparingForHome = false; // Zurücksetzen des Flags
    }
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
      currentState = ACTIVE;
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
//Display Anzeige
  if(isCharging) {
    laden();
  }
  if(kalib) {
    drawSmileyThinking();
  }
  else {
    fahren();
  }

//Webserver Input
    static unsigned long previousMillis = 0; // Speichert den letzten Zeitpunkt, zu dem der Akkustand aktualisiert wurde
    const long interval = 100; // Aktualisierungsintervall in Millisekunden (1 Sekunde)

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
          if (isCalibratingY) {
            yabmessung++; // Zähle hoch, wenn in Y-Kalibrierung
           }
          if (isCalibratingX) {
            xabmessung++; // Zähle hoch, wenn in X-Kalibrierung
          }

        
        
        
        
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

  switch (currentState) {
    
    //Kalibrierung
    case MOVING_FORWARDcalib:
      moveForward(); // Bewege dich vorwärts
      richtung = "oben";
      isCalibratingY = true; // Beginn der Y-Kalibrierung
      if (sensorValue == HIGH) { // Wenn der Sensor 1 ausgibt
          isCalibratingY = false;
          yabgemessen = true;
          currentState = TURNING_RIGHTcalib; // Wechsle den Zustand zu Rechtsabbiegung

        }

      break;

    case TURNING_RIGHTcalib:
      turnRight(); // Führe eine Rechtsabbiegung aus
        currentState = MOVING_FORWARD2calib;
      
      break;
      
    case MOVING_FORWARD2calib:
      moveForward();
      richtung = "rechts";
      isCalibratingX = true; // Beginn der X-Kalibrierung
      if (sensorValue==HIGH){
        isCalibratingX = false; // Ende der X-Kalibrierung
        isPreparingForHome = false;
        currentState= PREPARE_COMING_HOME_FROM_RIGHT;
      }
      
      break;


      //Hier Bedingung für Standardroute und Programmstart einfügen für Nachfolgendes
    case ACTIVE:
    if (roboterInBetrieb==true)
    {
      switch (aktuellerModus) {
                    case 1: // Beispiel für einen Modus
                        currentState = MOVING_START_FORWARD;
                        break;
                    case 6: // Kalibrierungsmodus
                        currentState = MOVING_FORWARDcalib;
                        break;
                    // Füge weitere Modi nach Bedarf hinzu
                    case 2:
                        currentState = MOVING_START_FORWARD;
                        break;
                    case 3:
                        currentState = MOVING_START_FORWARD;
                        break;
                    case 4:
                        currentState = MOVING_START_FORWARD;
                        break;
                    case 5:
                        currentState = MOVING_START_FORWARD;
                        break;                                        
                    default:
                        // Standardverhalten oder Fehlerbehandlung
                        break;
    }}

//Auf startposition fahren
    case MOVING_START_FORWARD:
      if (modus == 5){        //Wenn Modus = unten Rechts
        currentState = TURNING_RIGHT_START;
      } 
        else if ((modus=1)|(modus=2)|(modus=4)){    //wenn er ganz normal losfahren soll
        currentState=MOVING_FORWARD;
      }
      moveForward();
      aktuelly++;
      if(modus == 3 && aktuelly >= yabgemessen) {
        aktuelly = 0;
        currentState = TURNING_RIGHT_START;
         // Zurücksetzen, falls nötig
      }


      break;

    case TURNING_RIGHT_START:
      turnRight();
      currentState = MOVING_START_FORWARD2;
      break;

    case MOVING_START_FORWARD2:
      moveForward();
      aktuellx++;
      if((modus == 3 && aktuellx >= xabgemessen/2) || (modus == 5 && aktuellx >= xabgemessen/2)) {
        currentState = TURNING_LEFT_START;
        aktuellx = 0; // Zurücksetzen, falls nötig
      }
      break;

    case TURNING_LEFT_START:
      turnLeft();
      currentState = MOVING_FORWARD;
      break;
  

    case MOVING_FORWARD:
      moveForward(); // Bewege dich vorwärts
      aktuelly+=1;
      richtung = "oben";
      if (((sensorValue == HIGH)&(turnLeftNext))|((turnLeftNext)&((aktuelly==yabgemessen/2)&((modus==2)|(modus==3))))) { // Wenn der Sensor 1 ausgibt

          currentState = TURNING_LEFT; // Wechsle den Zustand zu Linksabbiegung
          turnLeftNext = false; // Setze zurück, damit das nächste Abbiegen wieder rechts ist
          aktuelly=0;
        }
      else if(((sensorValue==HIGH)&(modus==1|2|3))|((aktuelly>=yabgemessen)&(modus==4|5)))
      {
          currentState = TURNING_RIGHT; // Wechsle den Zustand zu Rechtsabbiegung
          aktuelly=0;
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
      richtung="rechts";
      aktuellx+1;
      if ((digitalRead(sensorPin) == HIGH)|(aktuellx==xabgemessen/2)) {
        currentState = STOPPINGOR;
      } else {
      currentState = TURNING_RIGHT2;
      }
      break;

    case TURNING_RIGHT2:
      turnRight(); // Führe die zweite Rechtsabbiegung aus
      currentState = MOVING_FORWARD;
      turnLeftNext = true; // Setze das Flag, damit beim nächsten Sensor HIGH links abgebogen wird
        richtung="unten";
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
      richtung="rechts";
      aktuellx+=1;
      if ((digitalRead(sensorPin) == HIGH)|(aktuellx==xabgemessen/2)){
        currentState = STOPPINGUR;
      } else {
      currentState = TURNING_LEFT2;
      }
      break;

    case TURNING_LEFT2:
      turnLeft();
      currentState = MOVING_FORWARD;
      break;


// Motor stoppt sobald er am Ende der Tafel angekommen ist
    case STOPPINGUR:
      stopMotors();
      delay(2000);
      currentState = ACTIVE;
      break;


    case HOMEUR:
    homeur();
    break;

    case STOPPINGOR:
      stopMotors();
      delay(2000);
      currentState = ACTIVE;
      break;


    case HOMEOR:
    homeor();
    break;

    case PREPARE_COMING_HOME_FROM_BOTTOM:
        turnRight(); // Drehe nach rechts
        currentState = HOMEOR;
        break;

    case PREPARE_COMING_HOME_FROM_TOP:
        turnLeft(); // Drehe nach links
        currentState = HOMEOR;
        break;

    case PREPARE_COMING_HOME_FROM_RIGHT:
        currentState = HOMEOR;
        break;
  }

    // Prüfe den Zustand des Stoppknopfes
  if (digitalRead(taster1) == HIGH) {
    // Entscheidung basierend auf der aktuellen Richtung
    if (richtung == "unten") {
      currentState = PREPARE_COMING_HOME_FROM_BOTTOM;
      isPreparingForHome = true;
    } else if (richtung == "oben") {
      currentState = PREPARE_COMING_HOME_FROM_TOP;
      isPreparingForHome = true;
    } else if (richtung == "rechts") {
      currentState = PREPARE_COMING_HOME_FROM_RIGHT;
    }
    // Verhindere weitere Aktionen in diesem Durchlauf
    return;
  }
  }