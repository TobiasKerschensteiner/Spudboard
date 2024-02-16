#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// WLAN-Zugangsdaten
const char* ssid = "Alpakhan";
const char* password = "Bananenmus";

WebServer server(80);
bool roboterInBetrieb = false;
int akkustand = 75; // Beispiel fuer einen Akkustand
int aktuellerModus = 0; // Globale Variable fuer den aktuellen Modus
int fortschritt = 0;
int verstricheneZeit;
bool aufStart = true; // 
unsigned long startZeitpunkt = 0; 
unsigned long stoppZeitpunkt = 0; 
const unsigned long wartezeit = 5000; // 5 Sekunden Wartezeit in Millisekunden
bool zurueckkehren = false; // Neue Zustandsvariable fuer "Zurueckkehren"
int modus;
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
            background-color: #f0f0f0; /* Für eine Hintergrundfarbe */
            /* Oder für ein Hintergrundbild: */
            /* background-image: url(''); */
            /* background-size: cover; */
            /* background-position: center; */
        }
        .button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }
        .tafel { width: 600px; height: 400px; border: 1px solid black; display: flex; flex-wrap: wrap; }
        .tafel button { width: 50%; height: 50%; }
        .battery { width: 50px; height: 20px; border: 1px solid black; position: relative; display: inline-block; vertical-align: middle; }
        .battery-level { height: 100%; background-color: green; }
        .battery::after { content: ''; position: absolute; top: 4px; right: -6px; width: 5px; height: 12px; border: 1px solid black; background-color: black; }
        .status-box { border: 1px solid black; padding: 10px; margin-top: 10px; }
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
    <div class='battery'><div class='battery-level' id="batteryLevel" style='width:75%;'></div></div>
    <span id="batteryText">Akku: 75%</span>
    
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
        setInterval(updateUI, 5000); // Aktualisiere die UI alle 5 Sekunden
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
        case 0: modusName = "Stopp"; break;
        case 1: modusName = "Standardroute"; break;
        case 2: modusName = "Oben Links";break;
        case 3: modusName = "Oben Rechts";break;
        case 4: modusName = "Unten Links";break;
        case 5: modusName = "Unten Rechts";break;
        case 6: modusName = "Kalibrieren";break;
        default: modusName = "Unbekannt"; break;
    }
    doc["modusName"] = modusName;

    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

void setup() {
    Serial.begin(9600);

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

void loop() {
    server.handleClient();
    if (roboterInBetrieb) {
    unsigned long aktuelleZeit = millis();
    verstricheneZeit = (aktuelleZeit - startZeitpunkt) / 1000; // Berechnet die verstrichene Zeit in Sekunden
    }
    // ueberprueft, ob die Wartezeit nach dem Stopp-Befehl abgelaufen ist
}