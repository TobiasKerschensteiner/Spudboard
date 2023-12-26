#include <Arduino.h>
#include <accelstepper.h>
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
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


int Sensor = 0; //Sensor = 0-> Sensor erkennt boden, Sensor = 1 -> Sensor erkennt keinen Boden
String Start = "off";
const int buttonPin = 0;
//ButtonPin = 1 Standardroute, 2 = Abmessen, 11= Links oben, 12= Rechts oben, 21= links unten, 22 = rechts unten
const int Drehkonstante = 100; //Wieviel es braucht um eine 90° Drehung zu machen
const int Groesse = 200; 

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


void loop(){




    if (buttonPin == 1){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if (Fahrmodus = 1){ 
            if(Sensor = 0){
            //fahr geradeaus hoch
            //GeradeausFunktion hier Vorkommen austauschen
                if (Standardyabgemessen=0){
                    Standardy+1;

            }
            if (Sensor = 1) //kommt an Oberer Kante an
            {
                Standardyabgemessen=1;
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
            }
        if(Fahrmodus = 12){
            if(ZaehlerDrehen!=Drehkonstante)
            {
                ZaehlerDrehen +1;
            //RechtsdrehenFunktion hier Vorkommen austauschen

            }
            if(ZaehlerDrehen=Drehkonstante){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        if(Fahrmodus = 2) { 
            if(Sensor=0){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
            //GeradeausFunktion hier Vorkommen austauschen
                aktuell+1;
                if (Standardxabgemessen=0){
                    Standardx+1;
                }
            }    
            if((Sensor=1)&&(letzteRunde=0)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
                if (Standardxabgemessen=0){
                    Standardxabgemessen=1;
                }           
            }
            if ((Sensor=1)&&(letzteRunde=1)){
                Fahrmodus=23;
            }
        if(Fahrmodus=23){
            if(ZaehlerDrehen!=2*Drehkonstante){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
            //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;

            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((Sensor=0)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
                aktuelly+1;
            }
            if((Sensor=0)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((Sensor=1)&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if(Fahrmodus=50){
            if(ZaehlerDrehen!=Drehkonstante){
                ZaehlerDrehen+1;
            //RechtsdrehenFunktion hier Vorkommen austauschen
            }
        }
            if(ZaehlerDrehen=Drehkonstante){
                ZaehlerDrehen=0;
                Fahrmodus=51;

        }
        if(Fahrmodus=51){
            if(Sensor=0){
                //GeradeausFunktion hier Vorkommen austauschen
            }
            if (Sensor=1){
                Fahrmodus=54;
            }
        }
        if(Fahrmodus=54){
            if(ZaehlerDrehen!=Drehkonstante){
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if(ZaehlerDrehen=Drehkonstante){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }

        }
        if(Fahrmodus=56){ 
            if(rueckfahrkonstante!=10)
            {
                //RueckwaertsFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if(Fahrmodus=3){
            if(Sensor=0){
                aktuell +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //faehrt gerade aus nach links
            }
            if((Sensor=1)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if(Fahrmodus=31){
            if(ZaehlerDrehen!=Drehkonstante){
                ZaehlerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if(ZaehlerDrehen=Drehkonstante){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if(Fahrmodus=43){ 
            if((aktuelly!=Standardy)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
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
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }

    if (buttonPin == 2){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if (Fahrmodus = 1){ 
            if(Sensor = 0){
                //fahr geradeaus hoch
                //GeradeausFunktion hier Vorkommen austauschen
                if (Standardyabgemessen=0){
                    Standardy+1;
                }
            }
            if((Sensor = 1)) //kommt an Oberer Kante an
            {
                Standardyabgemessen=1;
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
        }
        if(Fahrmodus = 12){ 
            if(ZaehlerDrehen!=Drehkonstante)
            {
                ZaehlerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen

            }
            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if (Sensor=0){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                //GeradeausFunktion hier Vorkommen austauschen
                if (Standardxabgemessen=0){
                    Standardx+1;
                }
            }
            if((Sensor=1)){        //kommt an rechter Wand an
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=2*Drehkonstante)){
                Fahrmodus =3;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if(Fahrmodus=3){
            if(Sensor=0){
                //GeradeausFunktion hier Vorkommen austauschen
                //faehrt gerade aus nach links
            }
            if((Sensor=1)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if(Fahrmodus=31){ 
            if(ZaehlerDrehen!=Drehkonstante){
                ZaehlerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=50;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if(Fahrmodus=50){
            if(aktuelly!=(Standardy-10)){
                aktuelly+1;
                //GeradeausFunktion hier Vorkommen austauschen
            }
            if((aktuelly=(Standardy-10))){
                aktuelly=0;
                Fahrmodus=55;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if(Fahrmodus=55){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
                //RueckwaertsFunktion hier Vorkommen austauschen
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
    if ((buttonPin == 11)&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";
        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if (Fahrmodus = 1){ 
            if((Sensor=0)){
                //fahr geradeaus hoch zur Startposition
                //GeradeausFunktion hier Vorkommen austauschen
            } 
            if ((Sensor=1)) //kommt an Oberer Kante an
            {
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
        }
        if(Fahrmodus = 12){
            if((ZaehlerDrehen!=Drehkonstante))
            {
                ZaehlerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }

            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if(aktuellx!=Standardx/2)
            {       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                //GeradeausFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((aktuelly!=(Standardy/2))&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if(Sensor=0){
                //GeradeausFunktion hier Vorkommen austauschen
            }
            if ((Sensor=1))
            {
                Fahrmodus=54;
            }
        }

        if(Fahrmodus=54){
            if((ZaehlerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=(Standardy/2)))
            {
                //RueckwaertsFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((Sensor=0)){
                aktuell +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //faehrt gerade aus nach links
            }
            if((Sensor=1)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((aktuelly!=Standardy/2)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
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
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }
        
    if ((buttonPin == 12)&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if ((Fahrmodus = 1)){
            if((aktuelly!=Standardy/2)){
                //fahr geradeaus hoch
                //GeradeausFunktion hier Vorkommen austauschen
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
            //RechtsdrehenFunktion hier Vorkommen austauschen

            }
            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if((Fahrmodus = 2)){
            if((aktuellx!=Standardx/2)){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                //GeradeausFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if((Fahrmodus=42)){
            if((Sensor=0)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
            }
            if((Sensor=0)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((Sensor=1)&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if((Fahrmodus=50)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen+1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if((Sensor=0)){
                //GeradeausFunktion hier Vorkommen austauschen
            }
            if((Sensor=1))
            {
                Fahrmodus=54;
            }
        }
        if((Fahrmodus=54)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=rueckfahrkonstante))
            {
                //RueckwaertsFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((Sensor=0)){
                aktuell +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //faehrt gerade aus nach links
            }
            if((Sensor=1)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((Sensor=0)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZaehlerFahren +1;
            }
            if((Sensor=1)&&(ZaehlerFahren!=Groesse)){
                letzteRunde=1; //faehrt ein letztes mal nach rechts
                Fahrmodus=32;
                ZaehlerFahren=0;
            }
            if((Sensor=0)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZaehlerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }

    if ((buttonPin == 12)&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if ((Fahrmodus = 1)){  
            if((aktuelly!=Standardy/2)){
                //fahr geradeaus hoch
                //GeradeausFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if((Fahrmodus = 2)){
            if((aktuellx!=Standardx/2)){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell+1;
                aktuellx+1;
            }
            if((aktuellx=Standardx/2)&&(letzteRunde=0)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
                aktuellx=0;
            }
            if ((aktuellx=Standardx/2)&&(letzteRunde=1)){
                Fahrmodus=23;
                aktuellx=0;
            }
        }
        if((Fahrmodus=23)){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if((Fahrmodus=42)){
            if((Sensor=0)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
            }
            if((Sensor=0)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((Sensor=1)&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if((Fahrmodus=50)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen+1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if((Sensor=0)){
                //GeradeausFunktion hier Vorkommen austauschen
            }
            if ((Sensor=1))
            {
            Fahrmodus=54;
            }
        }   
        if((Fahrmodus=54)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
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
                //RueckwaertsFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((Sensor=0)){
                aktuell +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //faehrt gerade aus nach links
            }
            if((Sensor=1)){
                aktuell=0;
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((Sensor=0)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZaehlerFahren +1;
            }
            if((Sensor=1)&&(ZaehlerFahren!=Groesse)){
                letzteRunde=1; //faehrt ein letztes mal nach rechts
                Fahrmodus=32;
                ZaehlerFahren=0;
            }
            if((Sensor=0)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZaehlerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }
if ((buttonPin == 21)&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";
        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if (Fahrmodus = 1){ 
            if((Sensor=0)){
                //fahr geradeaus hoch bis ende der Tafel
                //GeradeausFunktion hier Vorkommen austauschen
            } 
            if ((Sensor=1)) //kommt an Oberer Kante an
            {
                Fahrmodus= 12;           //initialisierung Rechtsdrehung
            }
        }
        if(Fahrmodus = 12){
            if((ZaehlerDrehen!=Drehkonstante))
            {
                ZaehlerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }

            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if(Sensor=0)
            {       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell+1;
            }

            if((Sensor=1)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
            }
            if((Sensor=1)&&(letzteRunde=1)){
                Fahrmodus=23;
            }
        }
        if(Fahrmodus=23){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((aktuelly!=(Standardy/2))&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if(Sensor=0){
                //GeradeausFunktion hier Vorkommen austauschen
            }
            if ((Sensor=1))
            {
                Fahrmodus=54;
            }
        }

        if(Fahrmodus=54){
            if((ZaehlerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=(Standardy/2)))
            {
                //RueckwaertsFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
                //GeradeausFunktion hier Vorkommen austauschen
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
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((aktuelly!=Standardy/2)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
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
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }
                
    if ((buttonPin == 22)&&(Standardxabgemessen=1)&&(Standardyabgemessen=1)){
            Start = "on";

        }

        if (Start == "on"){
            Fahrmodus = 1;
        }
        if ((Fahrmodus = 1)){  
            if((aktuelly!=Standardy/2)){
                //fahr geradeaus hoch 
                //GeradeausFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZaehlerDrehen =0;
            }
        }
        if((Fahrmodus = 2)){
            if((Sensor=0)){       //hat sich nach rechts gedreht und faehrt jetzt geradeaus
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell+1;
            }
            if((Sensor=1)&&(letzteRunde=0)){        //kommt an rechter Wand an
                Fahrmodus=21;           //Initialisierung Rechtsdrehung
            }
            if ((Sensor=1)&&(letzteRunde=1)){
                Fahrmodus=23;
            }
        }
        if((Fahrmodus=23)){
            if((ZaehlerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZaehlerDrehen +1;
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if((Fahrmodus=42)){
            if((Sensor=0)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZaehlerFahren +1;
                aktuell+1;
            }
            if((Sensor=0)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZaehlerFahren =0;
            }
            if((Sensor=1)&&(ZaehlerFahren!=Groesse)){
                Fahrmodus=50;
                ZaehlerFahren =0;
            }
        }
        if((Fahrmodus=50)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen+1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen=0;
                Fahrmodus=51;
            }
        }
        if((Fahrmodus=51)){
            if((Sensor=0)){
                //GeradeausFunktion hier Vorkommen austauschen
            }
            if ((Sensor=1))
            {
            Fahrmodus=54;
            }
        }   
        if((Fahrmodus=54)){
            if((ZaehlerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
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
                //RueckwaertsFunktion hier Vorkommen austauschen
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
                //RechtsdrehenFunktion hier Vorkommen austauschen
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
                //GeradeausFunktion hier Vorkommen austauschen
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
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((Sensor=0)&&(ZaehlerFahren!=Groesse)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZaehlerFahren +1;
            }
            if((Sensor=1)&&(ZaehlerFahren!=Groesse)){
                letzteRunde=1; //faehrt ein letztes mal nach rechts
                Fahrmodus=32;
                ZaehlerFahren=0;
            }
            if((Sensor=0)&&(ZaehlerFahren=Groesse)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZaehlerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZaehlerDrehen!=Drehkonstante)){
                ZaehlerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZaehlerDrehen=Drehkonstante)){
                ZaehlerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }
        }
        }
        }
}