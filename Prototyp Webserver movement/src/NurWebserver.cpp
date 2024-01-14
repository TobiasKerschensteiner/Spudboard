#include <Arduino.h>
#include <accelstepper.h>
#include <WiFi.h>
#include <FS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Server.h>

#include "html_template.h"

int Fahrmodus = 0; 
//0= stopp, 
//1= faehrt nach oben, 
//2= faehrt nach rechts, 
//3= faehrt nach links, 
//4= faehrt nach unten

//12= dreht sich rechts herum bei nach oben fahren,

//21=drehts sich nach rechts rechte Kante 1.Mal
//22=drehts sich nach rechts rechte Kante 2.Mal
//23=Komplettdrehung rechte Kante

//31= dreht sich nach links linke Kante 1.Mal
//32= dreht sich nach links linke Kante 2.Mal

//42= faehrt nach unten Rechte Kante
//43= Faehrt nach unten Linke Kante

//50= Bot dreht sich rechts um nach links um nachhause zu fahren


//51= Bot kommt von rechts und will Nachhause
//52= Bot will nachhause und ist an der linken Kante und muss nach unten fahren 
//54= Bot dreht sich nach rechts um dann Rueckwaerts zu fahren 
//55= Bot dreht sich um 180° bei Ladestation
//56= Bot faehrt rueckwaerts um anzudocken


int Sensorvalue = 0; //Sensorvalue = 0-> Sensorvalue erkennt boden, Sensorvalue = 1 -> Sensorvalue erkennt keinen Boden
String Start = "off";
String Einstellung = "off";
//different Options: "standardroute","Kalibrierung", "oben links","unten links","oben rechts","unten rechts"
const int Drehkonstante = 42; //Wieviel es braucht um eine 90° Drehung zu machen*100
const int Groesse = 500; //Breite der Bahn

int ZaehlerFahren = 0;
int ZaehlerDrehen = 0;

int Standardy =0; //Hoehe Tafel
int Standardyabgemessen =1; //Fraegt ab ob y abgemessen ist

int Standardx =0; //Breite Tafel
int Standardxabgemessen =1; //Fraegt ab ob x abgemessen ist

int aktuell =0; //gemachte Steps
int aktuelly=0;
int aktuellx=0;
int letzteRunde=0;
int rueckfahrkonstante=10;

int insgesamt= (Standardy/Groesse)*Standardx+Standardx;
double prozent = ((double)aktuell / insgesamt) * 100; //Prozentzahl bereitsgewischte Steps von Insgesamt

const char *ssid = "Alpakhan";
const char *password = "Bananenmus";

String header;
const unsigned long timeoutTime = 1000; // Timeout after 1 second
unsigned long currentTime = 0;
unsigned long previousTime = 0;
WiFiServer server(80);
#define MICROSTEP 16
int sensor = 15; //Sensorvalue = 0 -> sensor erkennt Boden, Sensorvalue = 1 -> Sensorvalue erkennt keinen Boden


//pin belegung
AccelStepper stepper1(MICROSTEP, 17, 18, 5, 16);  
AccelStepper stepper2(MICROSTEP, 26, 14, 27, 12);
//int sensor = 15; //Sensorvalue = 0 -> sensor erkennt Boden, Sensorvalue = 1 -> Sensorvalue erkennt keinen Boden
int gyroscl = 22;
int gyrosda = 21;
int maxstepperSpeed = 2000; //Max Geschwindigkeit soll nicht mehr als 2000 betragen
int stepperSpeed = 1000; //Geschwindigkeit
int turnSteps = 4200; //anzahl an schritten fuer eine 90° drehung 

//Variablen
//int maxstepperSpeed = 2000; //Max Geschwindigkeit soll nicht mehr als 2000 betragen
//int stepperSpeed = 1000; //Geschwindigkeit
//int turnSteps = 4200; //anzahl an schritten fuer eine 90° drehung 

void setup(){
  Serial.begin(9600);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
//90° Drehung nach rechts
void turnRight()
  {
    stepper1.move(100);
    stepper2.move(-100);

  }

//90° Drehung nach links
void turnLeft()
{
  stepper1.move(-100);
  stepper2.move(100);

}

//mini lauf nach unten (nach dem Bahnwechsel)
void moveswitch()
{
  stepper1.move(500);
  stepper2.move(500);


}

// Forwärtslauf fuers Abmessen mit merken und ausgabe
void moveForwab()
{
  stepper1.move(100);
  stepper2.move(100);

}

void moveBackward()
{
    stepper1.move(-100);
    stepper2.move(-100);
}
void loop(){
while(1){
    WiFiClient client = server.available();
    const char* myHtmlContent = htmlTemplate;
    if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

        if (header.indexOf("/Standardroute/") >= 0) {
              Serial.println("Standardroute An");
              Einstellung = "Standardroute";
              Start = "on";
              
            } else if (header.indexOf("/Zurueckfahren/") >= 0) {
              Serial.println("Stopp");
              Einstellung = "Zurueckfahren";
              Start = "off";
            } else if (header.indexOf("/Kalibrierung/") >= 0) {
              Serial.println("Kalibrierung An");
              Einstellung = "Kalibrierung";
              Start = "on";
            } else if (header.indexOf("/obenlinks/") >= 0) {
              Serial.println("oben links");
              Einstellung = "oben links";
              Start = "on";
            } else if (header.indexOf("/obenrechts/") >= 0) {
              Serial.println("oben rechts");
              Einstellung = "oben rechts";
              Start = "on";
            } else if (header.indexOf("/untenlinks/") >= 0) {
              Serial.println("unten links");
              Einstellung = "unten links";
              Start = "on";
            } else if (header.indexOf("/untenrechts/") >= 0) {
              Serial.println("unten rechts");
              Einstellung = "unten rechts";
              Start = "on";
            }
           String response = htmlTemplate;
            response += "<body><h1>PotatoWeb</h1>";
            response += "<p>Standardroute on </p>";

            if (Start== "off") {

              response += "<p><a href=\"/Standardroute/\"><button class=\"button\">ON</button></a></p>";
            }

            response += "<p>Kalibrierung on</p>";

            if (Start== "off") {

              response += "<p><a href=\"/Kalibrierung/\"><button class=\"button\">ON</button></a></p>";

            }

            response += "<p>oben links</p>";

            if ((Start== "off")&&(Standardxabgemessen=1)&(Standardyabgemessen=1)) {

              response += "<p><a href=\"/obenlinks/\"><button class=\"button\">ON</button></a></p>";

            }

            response += "<p>oben rechts</p>";

            if ((Start== "off")&&(Standardxabgemessen=1)&(Standardyabgemessen=1)) {

              response += "<p><a href=\"/obenrechts\"><button class=\"button\">ON</button></a></p>";
            }
            response += "<p>unten links</p>";

            if ((Start== "off")&&(Standardxabgemessen=1)&(Standardyabgemessen=1)) {

              response += "<p><a href=\"/untenlinks/\"><button class=\"button\">ON</button></a></p>";
            }
            response += "<p>unten rechts</p>";

            if ((Start== "off")&&(Standardxabgemessen=1)&(Standardyabgemessen=1)) {

              response += "<p><a href=\"/untenrechts\"><button class=\"button\">ON</button></a></p>";
            }
            if (Start == "on") {
              response += "<p><a href=\"/Zurueckfahren/\"><button class=\"button button2\">OFF</button></a></p>";
            }
            response += "</body></html>";
            client.println(response);
            break;
            }
            else {
            currentLine = "";
            }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
}

