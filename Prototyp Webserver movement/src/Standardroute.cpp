#include <Arduino.h>
#include <accelstepper.h>

int Fahrmodus = 0; 
//0= stopp, 1= fährt nach oben, 2= fährt nach rechts, 3= fährt nach links, 4= fährt nach unten
//12= dreht sich rechts herum bei nach oben fahren,21=drehts sich nach rechts 31= dreht sich nach links, 42= fährt nach unten Rechte Kante
//43= Fährt nach unten Linke Kante

int Sensor = 0; //Sensor = 0-> Sensor erkennt boden, Sensor = 1 -> Sensor erkennt keinen Boden
String Start = "off";
const int buttonPin = 0;



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
    if((Fahrmodus = 12) && (Sensor = 0 ))
    {
        //dreht sich nach rechts um 90°
        Fahrmodus =2; 
    }
    else if((Fahrmodus = 2) && (Sensor = 1)){       //kommt an rechter Wand an
        Fahrmodus =21;
    }
        












}