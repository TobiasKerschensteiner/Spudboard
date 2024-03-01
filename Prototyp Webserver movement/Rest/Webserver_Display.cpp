#include <Arduino.h>
//#include <Adafruit_GC9A01A.h>
//#include <Adafruit_GFX.h>
//#include <FS.h>
//#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <math.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>


/*
// Display-Pins definieren
#define TFT_CS   25  // Chip Select Pin
#define TFT_RST  27  // Reset Pin
#define TFT_DC   26  // Data/Command Pin
#define TFT_DIN  32
#define TFT_CLK  33
*/

// WLAN-Zugangsdaten
const char* ssid = "WLAN";
const char* password = "Password";

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



//Webserver
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

    server.send(200, "text/plain", "Modus gesetzt auf " + server.arg("modus"));
}
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
    </style>
</head>
<body>
    <h1>Tafelwischroboter Steuerung</h1>
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
    <script>
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
  Serial.println(akkustand);
  Serial.println(charging);
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
    Serial.begin(9600);

    // Initialisiere das Display
    tft.init();
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(4);
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    pinMode(13, INPUT_PULLDOWN);

    // Versuche, eine WiFi-Verbindung herzustellen
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        isWifiConnected = true;
        Serial.println("\nWiFi verbunden.");
        Serial.println("IP-Adresse: ");
        Serial.println(WiFi.localIP());
    } else {
        isWifiConnected = false;
        Serial.println("\nWiFi-Verbindung fehlgeschlagen.");
    }

    // Initialisiere den Timer und zeige ihn an
    remainingSeconds = totalSeconds;
    updateTimerDisplay();
}

void loop() {
  
  if(isCharging) {
    laden();
  }
  else {
    fahren();
  }

  unsigned long currentMillis = millis();
    // Akkustands-Update-Logik
    static unsigned long previousBatteryUpdateMillis = 0;
    const long batteryUpdateInterval = 1000;
    if (currentMillis - previousBatteryUpdateMillis >= batteryUpdateInterval) {
        previousBatteryUpdateMillis = currentMillis;
        getBattery();
    }

    // Roboter-Betriebs-Logik
    if (roboterInBetrieb) {
        unsigned long aktuelleZeit = millis();
        verstricheneZeit = (aktuelleZeit - startZeitpunkt) / 1000; // Berechnet die verstrichene Zeit in Sekunden
    }

    server.handleClient();
}
