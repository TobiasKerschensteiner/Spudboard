#include <Arduino.h>
#include <accelstepper.h>

int Fahrmodus = 0; 
//0= stopp, 
//1= fährt nach oben, 
//2= fährt nach rechts, 
//3= fährt nach links, 
//4= fährt nach unten

//12= dreht sich rechts herum bei nach oben fahren,

//21=drehts sich nach rechts rechte Kante 1.Mal
//22=drehts sich nach rechts rechte Kante 2.Mal

//31= dreht sich nach links linke Kante 1.Mal
//32= dreht sich nach links linke Kante 2.Mal

//42= fährt nach unten Rechte Kante
//43= Fährt nach unten Linke Kante

//50= Bot Fährt nachhause von Rechts


//51= Bot ist zuhause?



int Sensor = 0; //Sensor = 0-> Sensor erkennt boden, Sensor = 1 -> Sensor erkennt keinen Boden
String Start = "off";
const int buttonPin = 0;
const int Drehkonstante = 100; //Wieviel es braucht um eine 90° Drehung zu machen
const int Größe = 200;

int ZählerFahren = 0;
int ZählerDrehen = 0;

void loop(){

    if (buttonPin == 1){
        Start = "on";

    }

    if (Start == "on"){
        Fahrmodus = 1;
    }
    if ((Fahrmodus = 1) && (Sensor = 0)){
        //fahr geradeaus hoch

    }
    else if ((Fahrmodus = 1) && (Sensor = 1)) //kommt an Oberer Kante an
    {
        //dreht nach rechts um 90°
        Fahrmodus= 12; 
    }
    if((Fahrmodus = 12) && (ZählerDrehen!=Drehkonstante))
    {
        ZählerDrehen +1;
        //dreht sich nach rechts bis 90° erreicht

    }
    else if((Fahrmodus=12)&&(ZählerDrehen=Drehkonstante)){
        Fahrmodus =2;
        ZählerDrehen =0;
    }
    if((Fahrmodus = 2) && (Sensor=0)){       //hat sich nach rechts gedreht und fährt jetzt geradeaus
        //fährt gerade aus bis sensor =1

    }
    else if((Fahrmodus =2)&&(Sensor=1)){        //kommt an rechter Wand an
        //dreht sich nach unten
        Fahrmodus=21;
    }
    if((Fahrmodus=21)&&(ZählerDrehen!=Drehkonstante)){
        //dreht sich nach unten rechte Kante um Drehkonstante
        ZählerDrehen +1;

    }
    if((Fahrmodus=21)&&(ZählerDrehen=Drehkonstante)){
        ZählerDrehen=0;
        Fahrmodus=42;
    }
    if((Fahrmodus=42)&&(Sensor=0)&&(ZählerFahren!=Größe)){
        //fahr nach unten Rechte kante 
        ZählerFahren +1;
    }
    else if((Fahrmodus=42)&&(Sensor=0)&&(ZählerFahren=Größe)){
        Fahrmodus=22;
        ZählerFahren =0;
    }
    else if((Fahrmodus=42)&&(Sensor=1)&&(ZählerFahren!=Größe)){
        Fahrmodus=50;
        ZählerFahren =0;
    }  
    if((Fahrmodus=22)){
        //dreht sich um Drehkonstante nach rechts um nach links zu fahren
        Fahrmodus=3;
    }
    if((Fahrmodus=3)&&(Sensor=0)){
        //fährt gerade aus nach links
    }
    else if((Fahrmodus=3)&&(Sensor=1)){
        //dreht sich nach links an der linken Kante zum ersten mal
        Fahrmodus=31;
    }
    if((Fahrmodus=31)&&(ZählerDrehen!=Drehkonstante)){
        ZählerDrehen +1;
        //Drehen Funktion
    }
    else if((Fahrmodus=31)&&(ZählerDrehen=Drehkonstante)){
        ZählerDrehen =0;
        Fahrmodus=43;
    }
    if((Fahrmodus=43)&&(Sensor=0)&&(ZählerFahren!=Größe)){
        //FahrFunktion
        ZählerFahren +1;
    }
    else if((Fahrmodus=43)&&(Sensor=1)&&(ZählerFahren!=Größe)){
        Fahrmodus=51;
    }
    else if((Fahrmodus=43)&&(Sensor=0)&&(ZählerFahren=Größe)){
        Fahrmodus=32;
    }
    if((Fahrmodus=32)&&(ZählerDrehen!=Drehkonstante)){
        ZählerDrehen +1;
        //Drehen Funktion
    }
    else if((Fahrmodus=32)&&(ZählerDrehen=Drehkonstante)){
        ZählerDrehen =0;
        Fahrmodus=2;
    }
    if((Fahrmodus=50)&&(Sensor=0)){
        
    }
















}