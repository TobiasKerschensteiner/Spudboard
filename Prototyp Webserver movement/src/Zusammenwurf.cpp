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

// WLAN-Zugangsdaten
const char* ssid = "WLAN";
const char* password = "PASSWORD";

//#define HALFSTEP 8
#ifndef DRIVER
#define DRIVER 1
#endif
// Konstanten Roboter + Geschwindigkeiten 
const float stepsPerRevolution = 2048; // Schritte für eine volle Umdrehung im Half-Step-Modus
const float SteppDegree = 11.32;

const int maxSpeed = 2000; //Maximale Geschwindigkeit 
const int maxspeeddre = 1000; //Maximale Geschwindikeit bei den Drehungen 
const int besch = 200; // Beschleunigung
const int desiredSpeed = 1000; // Gewünschte Geschwindigkeit in Schritten pro Sekunde
const int delays = 2000; //delay von 2 sek
const int dist = 2000; //Kleiner versatzt von 1000 schritten 
const int Enable = 18;
// Pinbelegung Sonstige
const int taster1 = 34; //Taster stoppen 
const int taster2 = 35; // Taster Standardroute
const int sensorPin = 23; // Pin-Nummer des Sensors
const int wisch = 19; // Pin-Nummer wischmodul
//const int SCL = 23; //gryo
//const int SDA = 22; //gyro 

bool isPreparingForHome = false;

String richtung = "oben";

// Pin-Belegung für beide Stepper-Motoren
AccelStepper stepper1(DRIVER, 4,15);  
AccelStepper stepper2(DRIVER, 5,17);

//Standard
enum State {MOVING_FORWARD,
            TURNING_RIGHT, TURNING_RIGHT2,
            MOVING_SHORT_DISTANCER, MOVING_SHORT_DISTANCEL,
            TURNING_LEFT, TURNING_LEFT2,
            STOPPINGUR, HOMEUR, STOPPINGOR, HOMEOR,
            PREPARE_COMING_HOME_FROM_BOTTOM,
            PREPARE_COMING_HOME_FROM_TOP,
            PREPARE_COMING_HOME_FROM_RIGHT};
            
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
int akkustand = 0; // Beispiel fuer einen Akkustand
int aktuellerModus = 0; // Globale Variable fuer den aktuellen Modus
int fortschritt = 0;
int verstricheneZeit;
bool aufStart = true; // 
unsigned long startZeitpunkt = 0; 
unsigned long stoppZeitpunkt = 0; 
const unsigned long wartezeit = 5000; // 5 Sekunden Wartezeit in Millisekunden
bool zurueckkehren = false; // Neue Zustandsvariable fuer "Zurueckkehren"
int modus;

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

  akkustand = map(Voltage,2950,4095,0,100);

  //Serial.println(Voltage);
  Serial.println(akkustand);
}


void setup() {
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
});
    server.on("/getStatus", getStatus);
    server.begin();
    Serial.println("Webserver gestartet");
}

//Roboter machz eine 90° Drehung nach rechts
void turnRight()
{
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);
  
  float degree = 180;
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
  
  float degree = 180;
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
  
  float degree = 360;
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
//Webserver Input
    static unsigned long previousMillis = 0; // Speichert den letzten Zeitpunkt, zu dem der Akkustand aktualisiert wurde
    const long interval = 1000; // Aktualisierungsintervall in Millisekunden (1 Sekunde)

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        getBattery();
        /*if (roboterInBetrieb) {
            // Verringere den Akkustand nur, wenn der Roboter in Betrieb ist
            akkustand = akkustand > 0 ? akkustand - 1 : 100; // Verringere den Akkustand um 1% jede Sekunde, setze zurück auf 100% bei 0
            Serial.println("Akkustand: " + String(akkustand) + "%");
        }*/
    }
    server.handleClient();
    if (roboterInBetrieb) {
    unsigned long aktuelleZeit = millis();
    verstricheneZeit = (aktuelleZeit - startZeitpunkt) / 1000; // Berechnet die verstrichene Zeit in Sekunden
    }
    // ueberprueft, ob die Wartezeit nach dem Stopp-Befehl abgelaufen ist


  //Standardroute State 
  int sensorValue = digitalRead(sensorPin); // Lese den Sensorwert

  digitalWrite(wisch,HIGH); // wisch soll an gehen 

  switch (currentState) {
    case MOVING_FORWARD:
      moveForward(); // Bewege dich vorwärts
      richtung = "oben";
      if (sensorValue == HIGH) { // Wenn der Sensor 1 ausgibt
        if (turnLeftNext) {
          currentState = TURNING_LEFT; // Wechsle den Zustand zu Linksabbiegung
          turnLeftNext = false; // Setze zurück, damit das nächste Abbiegen wieder rechts ist
        } else {
          currentState = TURNING_RIGHT; // Wechsle den Zustand zu Rechtsabbiegung
        }
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