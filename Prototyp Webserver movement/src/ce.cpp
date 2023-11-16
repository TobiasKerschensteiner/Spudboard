/*
Hilfe:
https://cool-web.de/arduino/multi-function-shield-step-motor.htm
https://www.airspayce.com/mikem/arduino/AccelStepper/
*/

#include <Arduino.h>
#include <AccelStepper.h>

#define HALFSTEP 8

//pin belegung
AccelStepper stepper1(HALFSTEP, 17, 18, 5, 16);  
AccelStepper stepper2(HALFSTEP, 26, 14, 27, 12);

//Variablen
int maxstepperSpeed = 2000; //Max Geschwindigkeit soll nicht mehr als 2000 betragen
int stepperSpeed = 1000; //Geschwindigkeit
int turnSteps = 2100; //anzahl an schritten für eine 90° drehung 
int lineSteps = -2100; //Anzahl an schritte um gerade zu fahren 
int steps1 = 0; 
int steps2 = 0;

boolean turn1 = true; //track ob wir gerade fahren oder in einer Kurve
boolean turn2 = true;
boolean turn3 = true;
boolean turn4 = true;
boolean turn5 = true;
boolean turn6 = true;
boolean turn7 = true;
boolean turn8 = true;
boolean turn9 = true;
boolean turn10 = true;
 

void setup() {
  delay(3000); //Wartezeit 3sek

  //Motor 1
  stepper1.setMaxSpeed(maxstepperSpeed);
  stepper1.move(1); 
  stepper1.setSpeed(stepperSpeed);
  

  //Motor 2
  stepper2.setMaxSpeed(maxstepperSpeed);
  stepper2.move(-1);
  stepper2.setSpeed(stepperSpeed);
  

}

//Test Route
void loop()
{
  if(steps1 == 0 && steps2 == 0)
  {
    if (turn1) 
    {
      // 5000 Schritte gerade aus
      stepper1.move(5000);
      stepper2.move(5000);
    } 
    else if (turn2) 
    {
      // 90° Drehung nach rechts
      stepper1.move(turnSteps);
      stepper2.move(-turnSteps);
    } 
    else if (turn3) 
    {
      // 9000 Schritte gerade aus
      stepper1.move(9000);
      stepper2.move(9000);
    } 
    else if (turn4) 
    {
      // 90° Drehung nach rechts
      stepper1.move(turnSteps);
      stepper2.move(-turnSteps);
    } 
    else if (turn5) 
    {
      // 200 Schritte gerade aus
      stepper1.move(1000);
      stepper2.move(1000);
    } 
    else if (turn6) 
    {
      // 90° Drehung nach rechts
      stepper1.move(turnSteps);
      stepper2.move(-turnSteps);
    } 
    else if (turn7) 
    {
      // 9000 Schritte gerade aus
      stepper1.move(9000);
      stepper2.move(9000);
    } 
    else if (turn8) 
    {
      // 90° Drehung nach links
      stepper1.move(-turnSteps);
      stepper2.move(turnSteps);
    } 
    else if (turn9) 
    {
      // 200 Schritte gerade aus
      stepper1.move(1000);
      stepper2.move(1000);
    }
    else if (turn10)
    {
      // 90° Drehung nach links
      stepper1.move(-turnSteps);
      stepper2.move(turnSteps);
    }

    // Setzen der Geschwindigkeit
    stepper1.setSpeed(stepperSpeed);
    stepper2.setSpeed(stepperSpeed);


    // Togglen der Richtung für die nächste Iteration
    if (turn1) 
    {
      turn1 = false;
      turn2 = true;
    } 
    else if (turn2) 
    {
      turn2 = false;
      turn3 = true;
    } 
    else if (turn3) 
    {
      turn3 = false;
      turn4 = true;
    } 
    else if (turn4) 
    {
      turn4 = false;
      turn5 = true;
    } 
    else if (turn5) 
    {
      turn5 = false;
      turn6 = true;
    } 
    else if (turn6) 
    {
      turn6 = false;
      turn7 = true;
    } 
    else if (turn7) 
    {
      turn7 = false;
      turn8 = true;
    } 
    else if (turn8) 
    {
      turn8 = false;
      turn9 = true;
    } 
    else if (turn9) 
    {
      turn9 = false;
      turn10 = true;
    }
    else if (turn10)
    {
      turn10 = false;
    }
  }
  
  // Motoren zu den Zielpositionen bewegen
  stepper1.runSpeedToPosition();
  stepper2.runSpeedToPosition();

  // Aktualisieren der verbleibenden Schritte
  steps1 = stepper1.distanceToGo();
  steps2 = stepper2.distanceToGo();
}

