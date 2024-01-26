#include <WiFi.h>
#include <WebServer.h>

// Netzwerkinformationen
const char* ssid = "IhrNetzwerkname";
const char* password = "IhrPasswort";

WebServer server(80);

// Robotersteuerungsvariablen
int modus = 0;
const int roboterGroesse = 20;
bool roboterInBetrieb = false;

// Globale Variablen für die Dimensionen der Tafel
int tafelBreite = 0;
int tafelHoehe = 0;

// Motorsteuerungspins
const int motorLinksVorwaerts = D1;
const int motorRechtsVorwaerts = D2;

// Position und Ausrichtung
int aktuellePositionX = 0;
int aktuellePositionY = 0;
enum Ausrichtung { NORDEN, OSTEN, SUEDEN, WESTEN };
Ausrichtung aktuelleAusrichtung = SUEDEN;

// Akkustandsvariable
int akkustand = 100; // Beispielwert für Akkustand

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Verbindung zum WLAN...");
    }
    Serial.println("Verbunden mit WLAN!");

    server.on("/", handleRoot);
    server.on("/setModus", setModus);
    server.on("/stopp", handleStopp);
    server.begin();

    pinMode(motorLinksVorwaerts, OUTPUT);
    pinMode(motorRechtsVorwaerts, OUTPUT);
}



void handleRoot() {
    char html[4000];
    snprintf(html, 4000,
             "<html><head><title>Tafelwischroboter Steuerung</title><style>"
             ".button { background-color: #4CAF50; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }"
             ".tafel { width: 300px; height: 200px; border: 1px solid black; display: flex; flex-wrap: wrap; }"
             ".tafel button { width: 50%; height: 50%; }"
             ".battery { width: 50px; height: 20px; border: 1px solid black; position: relative; display: inline-block; vertical-align: middle; }"
             ".battery-level { height: 100%; background-color: green; width: %d%%; }"
             ".battery::after { content: ''; position: absolute; top: 4px; right: -6px; width: 5px; height: 12px; border: 1px solid black; background-color: black; }"
             ".status-box { border: 1px solid black; padding: 10px; margin-top: 10px; }"
             "</style></head><body>"
             "<h1>Tafelwischroboter Steuerung</h1>"
             "<button class='button' onclick=\"location.href='/setModus?modus=1'\">Standardroute</button>"
             "<button class='button' onclick=\"location.href='/setModus?modus=6'\">Kalibrieren</button>"
             "%s" // Platzhalter für Tafelviertel-Knöpfe
             "%s" // Platzhalter für den Stopp-Knopf
             "<div class='battery'><div class='battery-level' style='width:%d%%;'></div></div>"
             "<span>Akku: %d%%</span>"
             "<div class='status-box'>Status: %s</div>"
             "</body></html>",
             akkustand,
             tafelBreite > 0 && tafelHoehe > 0 ? "<div class='tafel'>"
             "<button onclick=\"location.href='/setModus?modus=2'\">Viertel 1</button>"
             "<button onclick=\"location.href='/setModus?modus=3'\">Viertel 2</button>"
             "<button onclick=\"location.href='/setModus?modus=4'\">Viertel 3</button>"
             "<button onclick=\"location.href='/setModus?modus=5'\">Viertel 4</button>"
             "</div>" : "",
             roboterInBetrieb ? "<button class='button' onclick=\"location.href='/stopp'\">Stopp</button>" : "",
             akkustand, akkustand, modus == 0 ? "Standby" : "Aktiv");
    server.send(200, "text/html", html);
}


void setModus() {
    if (server.hasArg("modus")) {
        int neuerModus = server.arg("modus").toInt();
        if (neuerModus >= 1 && neuerModus <= 6) {
            modus = neuerModus;
            roboterInBetrieb = true;
        }
        server.send(200, "text/plain", "Modus gesetzt auf " + String(modus));
    } else {
        server.send(400, "text/plain", "Fehlender Modus Parameter");
    }
}

void handleStopp() {
    modus = 0;
    zurueckZurStartposition();
    roboterInBetrieb = false;
    server.send(200, "text/plain", "Roboter gestoppt und zurückgesetzt");
}


void fahreVorwaerts(int schritte) {
    for (int i = 0; i < schritte; i++) {
        digitalWrite(motorLinksVorwaerts, HIGH);
        digitalWrite(motorRechtsVorwaerts, HIGH);
        delay(1000); // Dauer für einen Schritt, anpassen
        digitalWrite(motorLinksVorwaerts, LOW);
        digitalWrite(motorRechtsVorwaerts, LOW);
        delay(500); // Kurze Pause zwischen den Schritten

        // Aktualisiere die Position basierend auf der aktuellen Ausrichtung
        switch (aktuelleAusrichtung) {
            case NORDEN: aktuellePositionY++; break;
            case SUEDEN: aktuellePositionY--; break;
            case OSTEN: aktuellePositionX++; break;
            case WESTEN: aktuellePositionX--; break;
        }
    }
}

void dreheRechts() {
    // Beispiel: Nur den linken Motor aktivieren für eine Drehung
    digitalWrite(motorLinksVorwaerts, HIGH);
    delay(1000); // Dauer der Drehung, anpassen
    digitalWrite(motorLinksVorwaerts, LOW);

    // Aktualisiere die Ausrichtung beim Drehen
    aktuelleAusrichtung = static_cast<Ausrichtung>((aktuelleAusrichtung + 1) % 4);
}

bool pruefeKante() {
    // Beispiel: Verwenden eines fiktiven Kanten-Sensors
    // return digitalRead(kantenSensorPin) == HIGH;
    return false; // Standardmäßig false, bis implementiert
}

void kalibrieren() {
    // Fahre nach oben bis zur Kante
    while (!pruefeKante()) {
        fahreVorwaerts(1);
        tafelHoehe++;  // Zähle die Schritte
    }
    dreheRechts(); // Drehe nach Osten

    // Fahre nach rechts bis zur Kante
    while (!pruefeKante()) {
        fahreVorwaerts(1);
        tafelBreite++;  // Zähle die Schritte
    }

    // Zurück zur Startposition
    zurueckZurStartposition();
}
void fahreStandardroute() {

    while (!pruefeKante()) {
        fahreVorwaerts(1);
    }

    bool anDerKante = false;
    while (!anDerKante) {
        dreheRechts();
        fahreVorwaerts(roboterGroesse);
        if (pruefeKante()) {
            anDerKante = true;
            break;
        }
        dreheRechts();
        while (!pruefeKante()) {
            fahreVorwaerts(1);
        }
        dreheRechts();
        fahreVorwaerts(roboterGroesse);
        dreheRechts();
        while (!pruefeKante()) {
            fahreVorwaerts(1);
        }
    }
}

void zurueckZurStartposition() {
    // Drehe den Roboter zur linken Kante (Westen)
    while (aktuelleAusrichtung != WESTEN) {
        dreheRechts();
    }

    // Fahre zur Startposition zurück
    fahreVorwaerts(abs(aktuellePositionX));
    if (aktuellePositionY < 0) {
        dreheRechts(); // Norden
        fahreVorwaerts(abs(aktuellePositionY));
    } else {
        dreheLinks(); // Süden
        fahreVorwaerts(aktuellePositionY);
    }

    // Setze die Ausrichtung und Position zurück
    aktuelleAusrichtung = SUEDEN;
    aktuellePositionX = 0;
    aktuellePositionY = 0;
}
void wischeViertel(int start_x, int start_y, int breite, int hoehe) {
    // Setze den Roboter auf die Startposition des Viertels
    // Hier wird angenommen, dass der Roboter sich horizontal und vertikal bewegen kann
    // Bewegung zur Startposition (start_x, start_y)
    
    int ende_x = start_x + breite;
    int ende_y = start_y + hoehe;

    // Wische das Viertel in einem ähnlichen Muster wie die Standardroute
    for (int x = start_x; x < ende_x; x += roboterGroesse) {
        // Fahre vertikal nach oben
        for (int y = start_y; y < ende_y; y++) {
            // Bewegungslogik nach oben
        }
        // Fahre einen Schritt nach rechts
        // Bewegungslogik nach rechts

        // Fahre vertikal nach unten
        for (int y = ende_y; y > start_y; y--) {
            // Bewegungslogik nach unten
        }
        // Fahre einen Schritt nach rechts
        // Bewegungslogik nach rechts
    }
}

void wischeViertelObenLinks() {
    wischeViertel(0, 0, tafelBreite / 2, tafelHoehe / 2);
}

void wischeViertelObenRechts() {
    wischeViertel(tafelBreite / 2, 0, tafelBreite / 2, tafelHoehe / 2);
}

void wischeViertelUntenLinks() {
    wischeViertel(0, tafelHoehe / 2, tafelBreite / 2, tafelHoehe / 2);
}

void wischeViertelUntenRechts() {
    wischeViertel(tafelBreite / 2, tafelHoehe / 2, tafelBreite / 2, tafelHoehe / 2);
}
void loop() {
    server.handleClient();

    switch (modus) {
        case 1:  // Standardroute
            fahreStandardroute();
            break;
        case 2:  // Viertel oben links
            wischeViertelObenLinks();
            break;
        case 3:  // Viertel oben rechts
            wischeViertelObenRechts();
            break;
        case 4:  // Viertel unten links
            wischeViertelUntenLinks();
            break;
        case 5:  // Viertel unten rechts
            wischeViertelUntenRechts();
            break;
        case 6:  // Kalibrierung
            kalibrieren();
            break;
    }

    if (roboterInBetrieb) {
        // Hier können zusätzliche Aktionen hinzugefügt werden, wenn der Roboter aktiv ist.
    }

    modus = 0; // Setze den Modus zurück, nachdem die Aktion ausgeführt wurde
}
