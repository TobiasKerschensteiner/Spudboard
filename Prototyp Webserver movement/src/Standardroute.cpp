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
//23=Komplettdrehung rechte Kante

//31= dreht sich nach links linke Kante 1.Mal
//32= dreht sich nach links linke Kante 2.Mal

//42= fährt nach unten Rechte Kante
//43= Fährt nach unten Linke Kante

//50= Bot dreht sich rechts um nach links um nachhause zu fahren


//51= Bot kommt von rechts und will Nachhause
//52= Bot will nachhause und ist an der linken Kante und muss nach unten fahren 
//54= Bot dreht sich nach rechts um dann Rückwärts zu fahren 
//55= Bot dreht sich um 180° bei Ladestation
//56= Bot fährt rückwärts um anzudocken


int Sensor = 0; //Sensor = 0-> Sensor erkennt boden, Sensor = 1 -> Sensor erkennt keinen Boden
String Start = "off";
const int buttonPin = 0;
//ButtonPin = 1 Standardroute, 2 = Abmessen, 11= Links oben, 12= Rechts oben, 21= links unten, 22 = rechts unten
const int Drehkonstante = 100; //Wieviel es braucht um eine 90° Drehung zu machen
const int Größe = 200;

int ZählerFahren = 0;
int ZählerDrehen = 0;

int Standardy =0; //Höhe Tafel
int Standardyabgemessen =0; //Frägt ab ob y abgemessen ist

int Standardx =0; //Breite Tafel
int Standardxabgemessen =0; //Frägt ab ob x abgemessen ist

int aktuell =0; //gemachte Steps
int aktuelly=0;
int aktuellx=0;
int letzteRunde=0;
int rückfahrkonstante=10;
double prozent = ((double)aktuell / insgesamt) * 100; //Prozentzahl bereitsgewischte Steps von Insgesamt
int insgesamt= (Standardy/Größe)*Standardx+Standardx;

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
            if(ZählerDrehen!=Drehkonstante)
            {
                ZählerDrehen +1;
            //RechtsdrehenFunktion hier Vorkommen austauschen

            }
            if(ZählerDrehen=Drehkonstante){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZählerDrehen =0;
            }
        if(Fahrmodus = 2) { 
            if(Sensor=0){       //hat sich nach rechts gedreht und fährt jetzt geradeaus
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
            if(ZählerDrehen!=2*Drehkonstante){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
        
            if(ZählerDrehen=2*Drehkonstante){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurückfhren
            }        
        }
        if(Fahrmodus=21){
            if(ZählerDrehen!=Drehkonstante){
            //dreht sich nach unten rechte Kante um Drehkonstante
            //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;

            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((Sensor=0)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZählerFahren +1;
                aktuell+1;
                aktuelly+1;
            }
            if((Sensor=0)&&(ZählerFahren=Größe)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZählerFahren =0;
            }
            if((Sensor=1)&&(ZählerFahren!=Größe)){
                Fahrmodus=50;
                ZählerFahren =0;
            }
        }
        if(Fahrmodus=50){
            if(ZählerDrehen!=Drehkonstante){
                ZählerDrehen+1;
            //RechtsdrehenFunktion hier Vorkommen austauschen
            }
        }
            if(ZählerDrehen=Drehkonstante){
                ZählerDrehen=0;
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
            if(ZählerDrehen!=Drehkonstante){
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if(ZählerDrehen=Drehkonstante){
                ZählerDrehen=0;
                Fahrmodus=56;
            }

        }
        if(Fahrmodus=56){ 
            if(rückfahrkonstante!=10)
            {
                //RückwärtsFunktion hier Vorkommen austauschen
                rückfahrkonstante+1;
            }
            if((rückfahrkonstante=10)) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if(Fahrmodus=22){
            if(ZählerDrehen!=Drehkonstante){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu drehen
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if(Fahrmodus=3){
            if(Sensor=0){
                aktuell +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //fährt gerade aus nach links
            }
            if((Sensor=1)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if(Fahrmodus=31){
            if(ZählerDrehen!=Drehkonstante){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if(ZählerDrehen=Drehkonstante){
                ZählerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if(Fahrmodus=43){ 
            if((aktuelly!=Standardy)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZählerFahren +1;
            }
            if((aktuelly=Standardy)&&(ZählerFahren!=Größe)){
                letzteRunde=1; //fährt ein letztes mal nach rechts
                ZählerFahren=0;
                Fahrmodus=32;
            }        
            if((aktuelly!=Standardy)&&(ZählerFahren=Größe)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZählerFahren=0;
            }
        }
        if(Fahrmodus=32){ 
            if(ZählerDrehen!=Drehkonstante){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
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
            if(ZählerDrehen!=Drehkonstante)
            {
                ZählerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen

            }
            if((ZählerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZählerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if (Sensor=0){       //hat sich nach rechts gedreht und fährt jetzt geradeaus
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
            if(ZählerDrehen!=2*Drehkonstante)
            {
                ZählerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=2*Drehkonstante)){
                Fahrmodus =3;           //initialisierung nach rechts fahren
                ZählerDrehen =0;
            }
        }
        if(Fahrmodus=3){
            if(Sensor=0){
                //GeradeausFunktion hier Vorkommen austauschen
                //fährt gerade aus nach links
            }
            if((Sensor=1)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if(Fahrmodus=31){ 
            if(ZählerDrehen!=Drehkonstante){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;
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
            if((ZählerDrehen!=2*Drehkonstante)){
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen+1;
            }
            if((ZählerDrehen=2*Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=56;
            }
        }
        if(Fahrmodus=56){
            if((rückfahrkonstante!=10))
            {
                //RückwärtsFunktion hier Vorkommen austauschen
                rückfahrkonstante+1;
            }
        
            if((rückfahrkonstante=10)) //Roboter kommt an Ladestation an
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
            if((ZählerDrehen!=Drehkonstante))
            {
                ZählerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }

            if((ZählerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZählerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if(aktuellx!=Standardx/2)
            {       //hat sich nach rechts gedreht und fährt jetzt geradeaus
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
            if((ZählerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }

            if((ZählerDrehen=2*Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurückfhren
            }
        }  
        if(Fahrmodus=21){
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((aktuelly!=(Standardy/2))&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZählerFahren +1;
                aktuell+1;
                aktuelly+1;
            }
            if((aktuelly!=(Standardy/2))&&(ZählerFahren=Größe)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZählerFahren =0;
            }
            if((aktuelly=(Standardy/2))&&(ZählerFahren!=Größe)){
                Fahrmodus=50;
                ZählerFahren =0;
            }
        }
        if(Fahrmodus=50){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen+1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
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
            if((ZählerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=(Standardy/2)))
            {
                //RückwärtsFunktion hier Vorkommen austauschen
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
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((Sensor=0)){
                aktuell +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //fährt gerade aus nach links
            }
            if((Sensor=1)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((aktuelly!=Standardy/2)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZählerFahren +1;
            }
            if((aktuelly=Standardy/2)&&(ZählerFahren!=Größe)){
                letzteRunde=1; //fährt ein letztes mal nach rechts
                Fahrmodus=32;
                ZählerFahren=0;
            }
            if((aktuelly!=Standardy/2)&&(ZählerFahren=Größe)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZählerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
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
            if((ZählerDrehen!=Drehkonstante))
            {
            ZählerDrehen +1;
            //RechtsdrehenFunktion hier Vorkommen austauschen

            }
            if((ZählerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZählerDrehen =0;
            }
        }
        if((Fahrmodus = 2)){
            if((aktuellx!=Standardx/2)){       //hat sich nach rechts gedreht und fährt jetzt geradeaus
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
            if((ZählerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=2*Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurückfhren
            }
        } 
        if((Fahrmodus=21)){
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if((Fahrmodus=42)){
            if((Sensor=0)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZählerFahren +1;
                aktuell+1;
            }
            if((Sensor=0)&&(ZählerFahren=Größe)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZählerFahren =0;
            }
            if((Sensor=1)&&(ZählerFahren!=Größe)){
                Fahrmodus=50;
                ZählerFahren =0;
            }
        }
        if((Fahrmodus=50)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen+1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
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
            if((ZählerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=rückfahrkonstante))
            {
                //RückwärtsFunktion hier Vorkommen austauschen
                aktuelly+1;
            }
            if((aktuelly=rückfahrkonstante)) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if((Fahrmodus=22)){
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((Sensor=0)){
                aktuell +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //fährt gerade aus nach links
            }
            if((Sensor=1)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((Sensor=0)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZählerFahren +1;
            }
            if((Sensor=1)&&(ZählerFahren!=Größe)){
                letzteRunde=1; //fährt ein letztes mal nach rechts
                Fahrmodus=32;
                ZählerFahren=0;
            }
            if((Sensor=0)&&(ZählerFahren=Größe)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZählerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
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
            if((ZählerDrehen!=Drehkonstante))
            {
                ZählerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZählerDrehen =0;
            }
        }
        if((Fahrmodus = 2)){
            if((aktuellx!=Standardx/2)){       //hat sich nach rechts gedreht und fährt jetzt geradeaus
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
            if((ZählerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=2*Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurückfhren
            }
        } 
        if((Fahrmodus=21)){
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if((Fahrmodus=42)){
            if((Sensor=0)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZählerFahren +1;
                aktuell+1;
            }
            if((Sensor=0)&&(ZählerFahren=Größe)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZählerFahren =0;
            }
            if((Sensor=1)&&(ZählerFahren!=Größe)){
                Fahrmodus=50;
                ZählerFahren =0;
            }
        }
        if((Fahrmodus=50)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen+1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
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
            if((ZählerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen+1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=rückfahrkonstante))
            {
                //RückwärtsFunktion hier Vorkommen austauschen
                aktuelly+1;
            }
            if((aktuelly=rückfahrkonstante)) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if((Fahrmodus=22)){
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((Sensor=0)){
                aktuell +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //fährt gerade aus nach links
            }
            if((Sensor=1)){
                aktuell=0;
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((Sensor=0)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZählerFahren +1;
            }
            if((Sensor=1)&&(ZählerFahren!=Größe)){
                letzteRunde=1; //fährt ein letztes mal nach rechts
                Fahrmodus=32;
                ZählerFahren=0;
            }
            if((Sensor=0)&&(ZählerFahren=Größe)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZählerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
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
            if((ZählerDrehen!=Drehkonstante))
            {
                ZählerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }

            if((ZählerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZählerDrehen =0;
            }
        }
        if(Fahrmodus = 2){ 
            if(Sensor=0)
            {       //hat sich nach rechts gedreht und fährt jetzt geradeaus
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
            if((ZählerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }

            if((ZählerDrehen=2*Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurückfhren
            }
        }  
        if(Fahrmodus=21){
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if(Fahrmodus=42){
            if((aktuelly!=(Standardy/2))&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZählerFahren +1;
                aktuell+1;
                aktuelly+1;
            }
            if((aktuelly!=(Standardy/2))&&(ZählerFahren=Größe)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZählerFahren =0;
            }
            if((aktuelly=(Standardy/2))&&(ZählerFahren!=Größe)){
                Fahrmodus=50;
                ZählerFahren =0;
            }
        }
        if(Fahrmodus=50){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen+1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
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
            if((ZählerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=(Standardy/2)))
            {
                //RückwärtsFunktion hier Vorkommen austauschen
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
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((aktuelly!=Standardy/2)){
                aktuell +1;
                aktuelly+1;
                //GeradeausFunktion hier Vorkommen austauschen
                //fährt gerade aus nach links
            }
            if((aktuelly!=Standardy/2)){
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
                aktuelly=0;            
            }
        }
        if((Fahrmodus=31)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((aktuelly!=Standardy/2)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZählerFahren +1;
            }
            if((aktuelly=Standardy/2)&&(ZählerFahren!=Größe)){
                letzteRunde=1; //fährt ein letztes mal nach rechts
                Fahrmodus=32;
                ZählerFahren=0;
            }
            if((aktuelly!=Standardy/2)&&(ZählerFahren=Größe)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZählerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
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
            if((ZählerDrehen!=Drehkonstante))
            {
                ZählerDrehen +1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                Fahrmodus =2;           //initialisierung nach rechts fahren
                ZählerDrehen =0;
            }
        }
        if((Fahrmodus = 2)){
            if((Sensor=0)){       //hat sich nach rechts gedreht und fährt jetzt geradeaus
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
            if((ZählerDrehen!=2*Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante *2
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=2*Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Zurückfhren
            }
        } 
        if((Fahrmodus=21)){
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach unten rechte Kante um Drehkonstante
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=42;           //Initialisierung Rechte Kante runterfahren
            }
        }
        if((Fahrmodus=42)){
            if((Sensor=0)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                //fahr nach unten Rechte kante 
                ZählerFahren +1;
                aktuell+1;
            }
            if((Sensor=0)&&(ZählerFahren=Größe)){
                Fahrmodus=22;           //Initialisierung Rechte Kante zweites mal rechts abbiegen rechte Kante
                ZählerFahren =0;
            }
            if((Sensor=1)&&(ZählerFahren!=Größe)){
                Fahrmodus=50;
                ZählerFahren =0;
            }
        }
        if((Fahrmodus=50)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen+1;
                //RechtsdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
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
            if((ZählerDrehen!=Drehkonstante)){
                //LinksdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen+1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=56;
            }
        }
        if((Fahrmodus=56)){
            if((aktuelly!=rückfahrkonstante))
            {
                //RückwärtsFunktion hier Vorkommen austauschen
                aktuelly+1;
            }
            if((aktuelly=rückfahrkonstante)) //Roboter kommt an Ladestation an
            {
                aktuelly=0;
                aktuell=0;
                aktuellx=0;
                Start="off";
            }
        }
        if((Fahrmodus=22)){
            if((ZählerDrehen!=Drehkonstante)){
                //dreht sich nach rechts rechte Kante um Drehkonstante um nach links zu fahren 
                //RechtsdrehenFunktion hier Vorkommen austauschen
                ZählerDrehen +1;
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen=0;
                Fahrmodus=3;           //Initialisierung nach links fahren
            }
        }
        if((Fahrmodus=3)){
            if((aktuellx!=Standardx/2)){
                aktuell +1;
                aktuellx +1;
                //GeradeausFunktion hier Vorkommen austauschen
                //fährt gerade aus nach links
            }
            if((aktuellx=Standardx/2)){
                aktuellx=0;
                Fahrmodus=31;       // Initialisierung Linksdrehung an der linken Kante zum ersten mal
            }
        }
        if((Fahrmodus=31)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;
                Fahrmodus=43;       //Initialisierung Fahren an der liinken Kante 
            }
        }
        if((Fahrmodus=43)){
            if((Sensor=0)&&(ZählerFahren!=Größe)){
                //GeradeausFunktion hier Vorkommen austauschen
                aktuell +1;
                aktuelly +1;
                ZählerFahren +1;
            }
            if((Sensor=1)&&(ZählerFahren!=Größe)){
                letzteRunde=1; //fährt ein letztes mal nach rechts
                Fahrmodus=32;
                ZählerFahren=0;
            }
            if((Sensor=0)&&(ZählerFahren=Größe)){
                Fahrmodus=32;       //Initialisierung Linksdrehung an der linken Kante zum zweiten Mal
                ZählerFahren=0;
            }
        }
        if((Fahrmodus=32)){
            if((ZählerDrehen!=Drehkonstante)){
                ZählerDrehen +1;
                //LinksdrehenFunktion hier Vorkommen austauschen
            }
            if((ZählerDrehen=Drehkonstante)){
                ZählerDrehen =0;    //Initialisierung Rechtsfahren nachdem Links fertig gedreht wurde
                Fahrmodus=2;
            }
        }
        }
        }
        }
}