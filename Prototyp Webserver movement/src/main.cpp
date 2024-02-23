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

//#define HALFSTEP 8
#ifndef DRIVER
#define DRIVER 1
#endif
// Konstanten Roboter + Geschwindigkeiten 
const float stepsPerRevolution = 10800; // Schritte für eine volle Umdrehung im Half-Step-Modus
const float SteppDegree = 30;
const int maxSpeed = 10000; //Maximale Geschwindigkeit 
const int maxspeeddre = 10000; //Maximale Geschwindikeit bei den Drehungen 
const int besch = 4000; // Beschleunigung
const int desiredSpeed = 10000; // Gewünschte Geschwindigkeit in Schritten pro Sekunde
const int delays = 2000; //delay von 2 sek
const int dist = 5000; //Kleiner versatzt von 1000 schritten 
const int Enable = 18;
// Pinbelegung Sonstige
const int taster1 = 34; //Taster stoppen 
const int taster2 = 35; // Taster Standardroute
const int sensorPin = 23; // Pin-Nummer des Sensors
const int wisch = 19; // Pin-Nummer wischmodul
//const int SCL = 23; //gryo
//const int SDA = 22; //gyro 

// Pin-Belegung für beide Stepper-Motoren
AccelStepper stepper2(DRIVER, 4,15);  
AccelStepper stepper1(DRIVER, 5,17);

//Standard
enum State {MOVING_FORWARD,
            TURNING_RIGHT, TURNING_RIGHT2,
            MOVING_SHORT_DISTANCER, MOVING_SHORT_DISTANCEL,
            TURNING_LEFT, TURNING_LEFT2,
            STOPPINGUR, HOMEUR, STOPPINGOR, HOMEOR};
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


void setup() {
  // Initialisiere die Motoren mit der gewünschten Geschwindigkeit

  delay(4000); // Warten damit sich alle Signale beruhigen


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
}

//Roboter machz eine 90° Drehung nach rechts
void turnRight()
{
  stepper1.setMaxSpeed(maxspeeddre); // Erhöhe die Geschwindigkeit für die Drehung
  stepper1.setAcceleration(besch); // Erhöhe die Beschleunigung für eine schnellere Anlaufzeit
  stepper2.setMaxSpeed(maxspeeddre);
  stepper2.setAcceleration(besch);
  
  float degree = 2000;
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
  
  float degree = 2000;
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
  int sensorValue = digitalRead(sensorPin); // Lese den Sensorwert

  digitalWrite(wisch,HIGH); // wisch soll an gehen

  switch (currentState) {
    case MOVING_FORWARD:
      moveForward(); // Bewege dich vorwärts
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

  }
}