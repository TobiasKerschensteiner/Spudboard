#include <WiFi.h>
#include <WebServer.h>

// Netzwerkinformationen
const char* ssid = "Alpakhan";
const char* password = "Bananenmus";

WebServer server(80);

// Robotersteuerungsvariablen
int modus = 0;
const int roboterGroesse = 20;
bool roboterInBetrieb = false;

// Globale Variablen für die Dimensionen der Tafel
int tafelBreite = 0;
int tafelHoehe = 0;

int aktuellerFortschritt = 0; // Aktueller Fortschritt der Reinigung
int gesamtFlaeche = tafelBreite * tafelHoehe; // Berechnung der Gesamtfläche

// Definition der Motorsteuerungspins
const int ENABLEL = 18;
const int STEPL = 5;
const int DIRL = 17;

const int ENABLER = 18;
const int STEPR = 4;
const int DIRR = 15;

const int Sensor = 21;
const int Wisch = 19;

//Taster 
const int tasterPin = 2; // Ersetzen Sie 2 durch den tatsächlichen Pin


// Position und Ausrichtung
int steps = 10; // Anzahl der Schritte pro Bewegung
int aktuellePositionX = 0;
int aktuellePositionY = 0;
enum Ausrichtung { NORDEN, OSTEN, SUEDEN, WESTEN };
Ausrichtung aktuelleAusrichtung = SUEDEN;

// Akkustandsvariable
int akkustand = 100; // Beispielwert für Akkustand

void handleRoot();
void setModus();
void handleStopp();

void setup() {
    Serial.begin(9600);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Verbindung zum WLAN...");
    }
    Serial.println("Verbunden mit WLAN!");

    server.on("/", handleRoot);
    server.on("/setModus", setModus);
    server.on("/stopp", handleStopp);
    // IP-Adresse im Serial Monitor ausgeben
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
    server.begin();
    pinMode(Wisch, OUTPUT);
    pinMode(Sensor, INPUT);
    pinMode(ENABLEL, OUTPUT);
    pinMode(STEPL, OUTPUT);
    pinMode(DIRL, OUTPUT);
    digitalWrite(ENABLEL, LOW);

    pinMode(ENABLER, OUTPUT);
    pinMode(STEPR, OUTPUT);
    pinMode(DIRR, OUTPUT);
    digitalWrite(ENABLER, LOW);

    pinMode(tasterPin, INPUT_PULLUP);

}



void handleRoot() {
    // Initialisierung der HTML-Template-Teile
    char headerTemplate[] = "<html><head><title>Tafelwischroboter Steuerung</title><style>"
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
                            "<button class='button' onclick=\"location.href='/setModus?modus=6'\">Kalibrieren</button>";

    char tafelViertelButtonsTemplate[] = "<div class='tafel'>"
                                         "<button onclick=\"location.href='/setModus?modus=2'\">Viertel 1</button>"
                                         "<button onclick=\"location.href='/setModus?modus=3'\">Viertel 2</button>"
                                         "<button onclick=\"location.href='/setModus?modus=4'\">Viertel 3</button>"
                                         "<button onclick=\"location.href='/setModus?modus=5'\">Viertel 4</button>"
                                         "</div>";

    char stopButtonTemplate[] = "<button class='button' onclick=\"location.href='/stopp'\">Stopp</button>";

    char footerTemplate[] = "<div class='battery'><div class='battery-level' style='width:%d%%;'></div></div>"
                            "<span>Akku: %d%%</span>"
                            "<div class='status-box'>Status: %s</div>"
                            "</body></html>";

    // Bestimmung der Sichtbarkeit des Stopp-Knopfs und der Tafelviertel-Knöpfe basierend auf dem Betriebsstatus
    char stopButton[256] = "";
    if (roboterInBetrieb) {
        strcpy(stopButton, stopButtonTemplate);
    }

    int fortschrittsProzent = (aktuellerFortschritt * 100) / gesamtFlaeche; // Umwandlung in Prozent
    char fortschrittsAnzeige[100];
    snprintf(fortschrittsAnzeige, sizeof(fortschrittsAnzeige), "<div>Fortschritt: %d%%</div>", fortschrittsProzent);

    // Zusammenstellung des finalen HTML-Codes
    char html[8192];
    snprintf(html, sizeof(html), "%s%s%s<div class='battery'><div class='battery-level' style='width:%d%%;'></div></div><span>Akku: %d%%</span><div class='status-box'>Status: %s</div>%s</body></html>",
             headerTemplate,
             tafelViertelButtonsTemplate,
             stopButton,
             akkustand, 
             akkustand,
             modus == 0 ? "Standby" : "Aktiv",
             fortschrittsAnzeige);

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
void move(bool richtung) {
    digitalWrite(DIRL, richtung); // Richtung für linken Motor
    digitalWrite(DIRR, richtung); // Richtung für rechten Motor

    for (int stepCounter = 0; stepCounter < steps; stepCounter++) {
        digitalWrite(STEPL, HIGH);
        digitalWrite(STEPR, HIGH);
        delayMicroseconds(70);
        digitalWrite(STEPL, LOW);
        digitalWrite(STEPR, LOW);
        delayMicroseconds(70);
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
    // Drehe den Roboter nach rechts
    digitalWrite(DIRL, HIGH); // Ändere Richtung des linken Motors
    digitalWrite(DIRR, LOW);  // Beibehaltung der Richtung des rechten Motors
    move(false); // Ein Schritt für die Drehung
}

void dreheLinks() {
    // Drehe den Roboter nach links
    digitalWrite(DIRL, LOW);  // Beibehaltung der Richtung des linken Motors
    digitalWrite(DIRR, HIGH); // Ändere Richtung des rechten Motors
    move(false); // Ein Schritt für die Drehung
}

bool pruefeKante() {
    // Lese den Sensorwert und prüfe, ob eine Kante erreicht wurde
    return digitalRead(Sensor) == HIGH;
}

void fahreVorwaerts(int schritte) {
    for (int i = 0; i < schritte; i++) {
        move(true); // Vorwärtsbewegung
        digitalWrite(Wisch, HIGH); // Aktiviere den Wischmotor während der Bewegung
        delay(1000); // Pausiere kurz, um dem Wischmotor Zeit zum Arbeiten zu geben
    }
    digitalWrite(Wisch, LOW); // Deaktiviere den Wischmotor
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
void handleStopp() {
    modus = 0;
    zurueckZurStartposition();
    roboterInBetrieb = false;
    server.send(200, "text/plain", "Roboter gestoppt und zurückgesetzt");
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


void bewegeZuStartpositionViertel(int start_x, int start_y) {
    // Angenommen, der Roboter startet an der allgemeinen Startposition (0, 0)

    // Setze die Ausrichtung des Roboters
    aktuelleAusrichtung = OSTEN; // Richtung nach Osten für horizontale Bewegung

    // Fahre horizontal zur x-Position des Viertels
    while (aktuellePositionX < start_x) {
        fahreVorwaerts(1); // Ein Schritt nach rechts
    }

    // Ändere die Richtung nach Norden für vertikale Bewegung
    dreheLinks();
    aktuelleAusrichtung = NORDEN;

    // Fahre vertikal zur y-Position des Viertels
    while (aktuellePositionY < start_y) {
        fahreVorwaerts(1); // Ein Schritt nach oben
    }

    // Setze die Ausrichtung zurück
    aktuelleAusrichtung = OSTEN;
}

void zurueckZurAllgemeinenStartposition() {
    // Angenommen, der Roboter befindet sich an der Startposition eines Viertels

    // Setze die Ausrichtung des Roboters
    aktuelleAusrichtung = WESTEN; // Richtung nach Westen für horizontale Bewegung

    // Fahre horizontal zurück zur allgemeinen Startposition (0,0)
    while (aktuellePositionX > 0) {
        fahreVorwaerts(1); // Ein Schritt nach links
    }

    // Ändere die Richtung nach Süden für vertikale Bewegung
    dreheLinks();
    aktuelleAusrichtung = SUEDEN;

    // Fahre vertikal zurück zur allgemeinen Startposition (0,0)
    while (aktuellePositionY > 0) {
        fahreVorwaerts(1); // Ein Schritt nach unten
    }

    // Setze die Ausrichtung und Position zurück
    aktuelleAusrichtung = OSTEN;
    aktuellePositionX = 0;
    aktuellePositionY = 0;
}


void wischeViertel(int start_x, int start_y, int breite, int hoehe) {
    bewegeZuStartpositionViertel(start_x, start_y);
    // Bestimme die Endpositionen für das Viertel
    int ende_x = start_x + breite;
    int ende_y = start_y + hoehe;

    // Setze die anfängliche Richtung: Norden (nach oben)
    aktuelleAusrichtung = NORDEN;

    // Wische das Viertel in einem Zickzack-Muster
    for (int x = start_x; x < ende_x; x += roboterGroesse) {
        // Fahre vertikal durch das Viertel (nach oben, dann nach unten)
        for (int y = start_y; y < ende_y; y++) {
            fahreVorwaerts(1); // Ein Schritt nach oben
        }

        // Prüfe, ob noch Platz für eine horizontale Bewegung nach rechts ist
        if (x + roboterGroesse < ende_x) {
            dreheRechts(); // Ändere Richtung nach Osten
            fahreVorwaerts(1); // Ein Schritt nach rechts
            dreheLinks();  // Richtung wieder nach Norden ändern

            // Fahre zurück nach unten
            for (int y = ende_y; y > start_y; y--) {
                fahreVorwaerts(1); // Ein Schritt nach unten
            }

            // Prüfe erneut, ob Platz für eine weitere horizontale Bewegung nach rechts ist
            if (x + 2 * roboterGroesse < ende_x) {
                dreheRechts(); // Ändere Richtung nach Osten
                fahreVorwaerts(1); // Ein weiterer Schritt nach rechts
                dreheLinks();  // Richtung wieder nach Norden ändern
            }
        }
    }
    // Bewege den Roboter zurück zur allgemeinen Startposition
    zurueckZurAllgemeinenStartposition();
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

    // Tasterlogik und Modussteuerung
    static bool tasterGedrueckt = false;
    static unsigned long letzteTasterZeit = 0;
    const unsigned long entprellZeit = 50; // Entprellzeit in Millisekunden

    // Tasterstatus lesen und Entprellung
    if (digitalRead(tasterPin) == LOW && !tasterGedrueckt && millis() - letzteTasterZeit > entprellZeit) {
        tasterGedrueckt = true;
        letzteTasterZeit = millis();
        // Wechsle den Betriebsmodus
        if (!roboterInBetrieb) {
            modus = 1; // Standardroute als Beispiel
            roboterInBetrieb = true;
        } else {
            handleStopp();
        }
    } else if (digitalRead(tasterPin) == HIGH && tasterGedrueckt) {
        tasterGedrueckt = false;
    }

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