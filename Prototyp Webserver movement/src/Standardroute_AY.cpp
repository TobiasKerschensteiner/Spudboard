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
int Standardyabgemessen =0; //Fraegt ab ob y abgemessen ist

int Standardx =0; //Breite Tafel
int Standardxabgemessen =0; //Fraegt ab ob x abgemessen ist

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
int sensor = 15; //Sensorvalue = 0 -> sensor erkennt Boden, Sensorvalue = 1 -> Sensorvalue erkennt keinen Boden
int gyroscl = 22;
int gyrosda = 21;
int maxstepperSpeed = 2000; //Max Geschwindigkeit soll nicht mehr als 2000 betragen
int stepperSpeed = 1000; //Geschwindigkeit
int turnSteps = 4200; //anzahl an schritten für eine 90° drehung 

//Variablen
int maxstepperSpeed = 2000; //Max Geschwindigkeit soll nicht mehr als 2000 betragen
int stepperSpeed = 1000; //Geschwindigkeit
int turnSteps = 2100; //anzahl an schritten für eine 90° drehung 
int lineSteps = -2100; //Anzahl an Schrritte um gerade zu fahren
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

// Forwärtslauf fürs Abmessen mit merken und ausgabe
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
              
            } else if (header.indexOf("/Zurückfahren/") >= 0) {
              Serial.println("Stopp");
              Einstellung = "Zurückfahren";
              Start = "off";
            } else if (header.indexOf("/Kallibrierung/") >= 0) {
              Serial.println("Kalibrierung An");
              Einstellung = "Kalibrierung";

            } else if (header.indexOf("/oben links/") >= 0) {
              Serial.println("oben links");
              Einstellung = "oben links";

            } else if (header.indexOf("/oben rechts/") >= 0) {
              Serial.println("oben rechts");

              Einstellung = "oben rechts";

            } else if (header.indexOf("/unten links/") >= 0) {
              Serial.println("unten links");
              Einstellung = "unten links";

            } else if (header.indexOf("/unten rechts/") >= 0) {
              Serial.println("unten rechts");
              Einstellung = "unten rechts";

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

              response += "<p><a href=\"/oben links/\"><button class=\"button\">ON</button></a></p>";

            }

            response += "<p>oben rechts</p>";

            if ((Start== "off")&&(Standardxabgemessen=1)&(Standardyabgemessen=1)) {

              response += "<p><a href=\"/oben rechts\"><button class=\"button\">ON</button></a></p>";
            }
            response += "<p>unten links</p>";

            if ((Start= "off")&&(Standardxabgemessen=1)&(Standardyabgemessen=1)) {

              response += "<p><a href=\"/unten links/\"><button class=\"button\">ON</button></a></p>";
            }
            response += "<p>unten rechts</p>";

            if ((Start== "off")&&(Standardxabgemessen=1)&(Standardyabgemessen=1)) {

              response += "<p><a href=\"/unten rechts\"><button class=\"button\">ON</button></a></p>";
            }
            if (Start == "on") {
              response += "<p><a href=\"/Zurückfahren/\"><button class=\"button button2\">OFF</button></a></p>";
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


    if (Einstellung == "Standardroute"){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if (Fahrmodus = 1){ 
            if(Sensorvalue = 0){
            //fahr geradeaus hoch
            moveForwab();
                if (Standardyabgemessen=0){
                    Standardy+1;

            }
            if (Sensorvalue = 1) //kommt an Oberer Kante an
            {
                Standardyabgemessen=1;
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
            }
        if(Fahrmodus = 12){
            if(ZaehlerDrehen!=Drehkonstante)
            {
                ZaehlerDrehen +1;
            turnRight();

            }
            if(ZaehlerDrehen=Drehkonstante){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        if(Fahrmodus = 2) { 
            if(Sensorvalue=LOW){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
            moveForwab();
                aktuell+1;
                if (Standardxabgemessen=0){
                    Standardx+1;
                }
            }    
            if((Sensorvalue=HIGH)&&(letzteRunde=0)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
                if (Standardxabgemessen=0){
                    Standardxabgemessen=1;
                }           
            }
            if ((Sensorvalue=HIGH)&&(letzteRunde=1)){
                Fahrmodus=23;
            }}
        if(Fahrmodus=23){
            if(ZaehlerDrehen!=2*Drehkonstante){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                turnRight();
                ZaehlerDrehen +1;
            }
        
            if(ZaehlerDrehen=2*Drehkonstante){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurueckfhren
            }        
        }
        if(Fahrmodus=21){
            if(ZaehlerDrehen!=Drehkonstante){
            //dreht sich nach unten rechte Kante um Drehkonstante
            turnRight();
                ZaehlerDrehen +1;

            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((Sensorvalue=LOW)&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
                aktuelly+1;
            }
            if((Sensorvalue=LOW)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((Sensorvalue=HIGH)&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if(Fahrmodus=50){
            if(ZaehlerDrehen!=Drehkonstante){
                ZaehlerDrehen+1;
            turnRight();
            }
        }
            if(ZaehlerDrehen=Drehkonstante){
                ZaehlerDrehen=0;
                Fahrmodus=51;

        }
        if(Fahrmodus=51){
            if(Sensorvalue=LOW){
                moveForwab();
            }
            if (Sensorvalue=HIGH){
                Fahrmodus=54;
            }
        }
        if(Fahrmodus=54){
            if(ZaehlerDrehen!=Drehkonstante){
                turnRight();
            }
            if(ZaehlerDrehen=Drehkonstante){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }

        }
        if(Fahrmodus=56){ 
            if(rueckfahrkonstante!=10)
            {
                moveBackward();
                rueckfahrkonstante+1;
            }
            if((rueckfahrkonstante=10)) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if(Fahrmodus=22){
            if(ZaehlerDrehen!=Drehkonstante){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu drehen
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if(Fahrmodus=3){
            if(Sensorvalue=LOW){
                aktuell +1;
                moveForwab();
                //faehrt gerade aus nach links
            }
            if((Sensorvalue=HIGH)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if(Fahrmodus=31){
            if(ZaehlerDrehen!=Drehkonstante){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if(ZaehlerDrehen=Drehkonstante){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if(Fahrmodus=43){ 
            if((aktuelly!=Standardy)&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                aktuell +1;
                aktuelly +1;
                ZaehlerFahren +1;
            }
            if((aktuelly=Standardy)&&(ZaehlerFahren!=Groesse)){
                letzteRunde=1; //faehrt ein letztes mal nach rechts
                ZaehlerFahren=0;
                Fahrmodus=32;
            }        
            if((aktuelly!=Standardy)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZaehlerFahren=0;
            }
        }
        if(Fahrmodus=32){ 
            if(ZaehlerDrehen!=Drehkonstante){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }

    if (Einstellung == "Kalibrierung"){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if (Fahrmodus = 1){ 
            if(Sensorvalue = 0){
                //fahr geradeaus hoch
                moveForwab();
                if (Standardyabgemessen=0){
                    Standardy+1;
                }
            }
            if((Sensorvalue = 1)) //kommt an Oberer Kante an
            {
                Standardyabgemessen=1;
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
        }
        if(Fahrmodus = 12){ 
            if(ZaehlerDrehen!=Drehkonstante)
            {
                ZaehlerDrehen +1;
                turnRight();

            }
            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if (Sensorvalue=LOW){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                moveForwab();
                if (Standardxabgemessen=0){
                    Standardx+1;
                }
            }
            if((Sensorvalue=HIGH)){        //kommt an rechter Wand an
                Fahrmodus=23;           //Initialisierung Rechtsdrehung
                if (Standardxabgemessen=0){
                    Standardxabgemessen=1;
                }           
            }
        }
        if(Fahrmodus = 23){ 
            if(ZaehlerDrehen!=2*Drehkonstante)
            {
                ZaehlerDrehen +1;
                turnRight();
            }
            if((ZaehlerDrehen=2*Drehkonstante)){
                Fahrmodus =3;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if(Fahrmodus=3){
            if(Sensorvalue=LOW){
                moveForwab();
                //faehrt gerade aus nach links
            }
            if((Sensorvalue=HIGH)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if(Fahrmodus=31){ 
            if(ZaehlerDrehen!=Drehkonstante){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=50;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if(Fahrmodus=50){
            if(aktuelly!=(Standardy-10)){
                aktuelly+1;
                moveForwab();
            }
            if((aktuelly=(Standardy-10))){
                aktuelly=0;
                Fahrmodus=55;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if(Fahrmodus=55){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                turnRight();
                ZaehlerDrehen+1;
            }
            if((ZaehlerDrehen=2*Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }
        }
        if(Fahrmodus=56){
            if((rueckfahrkonstante!=10))
            {
                moveBackward();
                rueckfahrkonstante+1;
            }
        
            if((rueckfahrkonstante=10)) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";


            }
        }
    if ((Einstellung == "oben links")&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";
        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if (Fahrmodus = 1){ 
            if((Sensorvalue=LOW)){
                //fahr geradeaus hoch zur Startposition
                moveForwab();
            } 
            if ((Sensorvalue=HIGH)) //kommt an Oberer Kante an
            {
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
        }
        if(Fahrmodus = 12){
            if((ZaehlerDrehen!=Drehkonstante))
            {
                ZaehlerDrehen +1;
                turnRight();
            }

            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if(aktuellx!=Standardx/2)
            {       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                moveForwab();
                aktuell+1;
                aktuellx+1;
            }

            if((aktuellx=Standardx/2)&&(letzteRunde=0)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
                aktuellx=0;
            }
            if((aktuellx=Standardx/2)&&(letzteRunde=1)){
                aktuellx=0;
                Fahrmodus=23;
            }
        }
        if(Fahrmodus=23){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                turnRight();
                ZaehlerDrehen +1;
            }

            if((ZaehlerDrehen=2*Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurueckfhren
            }
        }  
        if(Fahrmodus=21){
            if((ZaehlerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((aktuelly!=(Standardy/2))&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
                aktuelly+1;
            }
            if((aktuelly!=(Standardy/2))&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((aktuelly=(Standardy/2))&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if(Fahrmodus=50){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen+1;
                turnRight();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if(Sensorvalue=LOW){
                moveForwab();
            }
            if ((Sensorvalue=HIGH))
            {
                Fahrmodus=54;
            }
        }

        if(Fahrmodus=54){
            if((ZaehlerDrehen!=Drehkonstante)){
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=(Standardy/2)))
            {
                moveBackward();
                aktuelly+1;
            }
            if((aktuelly=(Standardy/2))) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if((Fahrmodus=22)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((Sensorvalue=LOW)){
                aktuell +1;
                moveForwab();
                //faehrt gerade aus nach links
            }
            if((Sensorvalue=HIGH)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((aktuelly!=Standardy/2)&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                aktuell +1;
                aktuelly +1;
                ZaehlerFahren +1;
            }
            if((aktuelly=Standardy/2)&&(ZaehlerFahren!=Groesse)){
                letzteRunde=1; //faehrt ein letztes mal nach rechts
                Fahrmodus=32;
                ZaehlerFahren=0;
            }
            if((aktuelly!=Standardy/2)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZaehlerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }
        
    if ((Einstellung == "unten links")&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if ((Fahrmodus = 1)){
            if((aktuelly!=Standardy/2)){
                //fahr geradeaus hoch
                moveForwab();
            }
            if ((aktuelly=Standardy/2)) //kommt an Oberer Kante an
            {
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
        }
        if((Fahrmodus = 12)){
            if((ZaehlerDrehen!=Drehkonstante))
            {
            ZaehlerDrehen +1;
            turnRight();

            }
            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if((Fahrmodus = 2)){
            if((aktuellx!=Standardx/2)){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                moveForwab();
                aktuell+1;
                aktuellx+1;            
                }
            if((aktuellx=Standardx/2)&&(letzteRunde=0)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
                aktuellx=0;
            }
            if ((aktuellx=Standardx/2)&&(letzteRunde=1)){
                Fahrmodus=23;
            }
        }
        if((Fahrmodus=23)){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=2*Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurueckfhren
            }
        } 
        if((Fahrmodus=21)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if((Fahrmodus=42)){
            if((Sensorvalue=LOW)&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
            }
            if((Sensorvalue=LOW)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((Sensorvalue=HIGH)&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if((Fahrmodus=50)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen+1;
                turnRight();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if((Sensorvalue=LOW)){
                moveForwab();
            }
            if((Sensorvalue=HIGH))
            {
                Fahrmodus=54;
            }
        }
        if((Fahrmodus=54)){
            if((ZaehlerDrehen!=Drehkonstante)){
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=rueckfahrkonstante))
            {
                moveBackward();
                aktuelly+1;
            }
            if((aktuelly=rueckfahrkonstante)) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if((Fahrmodus=22)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((Sensorvalue=LOW)){
                aktuell +1;
                moveForwab();
                //faehrt gerade aus nach links
            }
            if((Sensorvalue=HIGH)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((Sensorvalue=LOW)&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                aktuell +1;
                aktuelly +1;
                ZaehlerFahren +1;
            }
            if((Sensorvalue=HIGH)&&(ZaehlerFahren!=Groesse)){
                letzteRunde=1; //faehrt ein letztes mal nach rechts
                Fahrmodus=32;
                ZaehlerFahren=0;
            }
            if((Sensorvalue=LOW)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZaehlerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }

    
if ((Einstellung == "oben rechts")&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";
        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if (Fahrmodus = 1){ 
            if((Sensorvalue=LOW)){
                //fahr geradeaus hoch bis ende der Tafel
                moveForwab();
            } 
            if ((Sensorvalue=HIGH)) //kommt an Oberer Kante an
            {
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
        }
        if(Fahrmodus = 12){
            if((ZaehlerDrehen!=Drehkonstante))
            {
                ZaehlerDrehen +1;
                turnRight();
            }

            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if(Sensorvalue=LOW)
            {       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                moveForwab();
                aktuell+1;
            }

            if((Sensorvalue=HIGH)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
            }
            if((Sensorvalue=HIGH)&&(letzteRunde=1)){
                Fahrmodus=23;
            }
        }
        if(Fahrmodus=23){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                turnRight();
                ZaehlerDrehen +1;
            }

            if((ZaehlerDrehen=2*Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurueckfhren
            }
        }  
        if(Fahrmodus=21){
            if((ZaehlerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((aktuelly!=(Standardy/2))&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
                aktuelly+1;
            }
            if((aktuelly!=(Standardy/2))&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((aktuelly=(Standardy/2))&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if(Fahrmodus=50){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen+1;
                turnRight();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if(Sensorvalue=LOW){
                moveForwab();
            }
            if ((Sensorvalue=HIGH))
            {
                Fahrmodus=54;
            }
        }

        if(Fahrmodus=54){
            if((ZaehlerDrehen!=Drehkonstante)){
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=(Standardy/2)))
            {
                moveBackward();
                aktuelly+1;
            }
            if((aktuelly=(Standardy/2))) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if((Fahrmodus=22)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((aktuelly!=Standardy/2)){
                aktuell +1;
                aktuelly+1;
                moveForwab();
                //faehrt gerade aus nach links
            }
            if((aktuelly!=Standardy/2)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
                aktuelly=0;            
            }
        }
        if((Fahrmodus=31)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((aktuelly!=Standardy/2)&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                aktuell +1;
                aktuelly +1;
                ZaehlerFahren +1;
            }
            if((aktuelly=Standardy/2)&&(ZaehlerFahren!=Groesse)){
                letzteRunde=1; //faehrt ein letztes mal nach rechts
                Fahrmodus=32;
                ZaehlerFahren=0;
            }
            if((aktuelly!=Standardy/2)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZaehlerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }
                
    if ((Einstellung == "unten rechts")&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if ((Fahrmodus = 1)){  
            if((aktuelly!=Standardy/2)){
                //fahr geradeaus hoch 
                moveForwab();
            }
            if ((aktuelly=Standardy/2)) //kommt an Oberer Kante an
            {
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
        }
        if((Fahrmodus = 12)){
            if((ZaehlerDrehen!=Drehkonstante))
            {
                ZaehlerDrehen +1;
                turnRight();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if((Fahrmodus = 2)){
            if((Sensorvalue=LOW)){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                moveForwab();
                aktuell+1;
            }
            if((Sensorvalue=HIGH)&&(letzteRunde=0)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
            }
            if ((Sensorvalue=HIGH)&&(letzteRunde=1)){
                Fahrmodus=23;
            }
        }
        if((Fahrmodus=23)){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=2*Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurueckfhren
            }
        } 
        if((Fahrmodus=21)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if((Fahrmodus=42)){
            if((Sensorvalue=LOW)&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
            }
            if((Sensorvalue=LOW)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((Sensorvalue=HIGH)&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if((Fahrmodus=50)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen+1;
                turnRight();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if((Sensorvalue=LOW)){
                moveForwab();
            }
            if ((Sensorvalue=HIGH))
            {
            Fahrmodus=54;
            }
        }   
        if((Fahrmodus=54)){
            if((ZaehlerDrehen!=Drehkonstante)){
                turnLeft();
                ZaehlerDrehen+1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=rueckfahrkonstante))
            {
                moveBackward();
                aktuelly+1;
            }
            if((aktuelly=rueckfahrkonstante)) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if((Fahrmodus=22)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                turnRight();
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((aktuellx!=Standardx/2)){
                aktuell +1;
                aktuellx +1;
                moveForwab();
                //faehrt gerade aus nach links
            }
            if((aktuellx=Standardx/2)){
                aktuellx=0;
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((Sensorvalue=LOW)&&(ZaehlerFahren!=Groesse)){
                moveForwab();
                aktuell +1;
                aktuelly +1;
                ZaehlerFahren +1;
            }
            if((Sensorvalue=HIGH)&&(ZaehlerFahren!=Groesse)){
                letzteRunde=1; //faehrt ein letztes mal nach rechts
                Fahrmodus=32;
                ZaehlerFahren=0;
            }
            if((Sensorvalue=LOW)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZaehlerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                turnLeft();
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }

        }
        }
        }
