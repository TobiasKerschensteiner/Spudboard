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
int maxstepperSpeed = 10000; //Max Geschwindigkeit soll nicht mehr als 2000 betragen
int stepperSpeed = 10000; //Geschwindigkeit
int stepperbe = 5000;
int turnSteps = 2100; //anzahl an schritten für eine 90° drehung 
int lineSteps = -2100; //Anzahl an schritte um gerade zu fahren 
int steps1 = 0; 
int steps2 = 0;
int sensor = 15;

boolean turn1 = true; //track ob wir gerade fahren oder in einer Kurve
boolean turn2 = true;
 

void setup() 
{
  Serial.begin(9600);
  delay(3000); //Wartezeit 3sek

  //Motor 1
  stepper1.setMaxSpeed(maxstepperSpeed);
  stepper1.setAcceleration(stepperbe);
  stepper1.move(1); 
  stepper1.setSpeed(stepperSpeed);
  
  

  //Motor 2
  stepper2.setMaxSpeed(maxstepperSpeed);
  stepper2.setAcceleration(stepperbe);
  stepper2.move(-1);
  stepper2.setSpeed(stepperSpeed);

}


//90° Drehung nach rechts
void turnRight()
  {
    stepper1.move(turnSteps);
    stepper2.move(-turnSteps);

  }


//Test Route

void loop()
{
  // Sensorstatus abfragen
  int sensorValue = digitalRead(sensor);

  Serial.print("Sensor: ");
  Serial.println(sensorValue);
  // Setzen der Geschwindigkeit
  stepper1.setSpeed(stepperSpeed);
  stepper1.setAcceleration(stepperbe);
  stepper2.setSpeed(stepperSpeed);
  stepper2.setAcceleration(stepperbe);

  
  if (sensorValue == 0)
  {
    // Überprüfen, ob beide Motoren an ihrem Ziel sind
    if (steps1 == 0 && steps2 == 0)
    {
      // Motoren stoppen
      stepper1.stop();
      stepper2.stop();
    }
  }

  if (sensorValue == 1)
  {
    // Überprüfen, ob beide Motoren gestoppt sind
    if (!stepper1.isRunning() && !stepper2.isRunning())
    {
      // Je nach dem, ob wir geradeaus fahren oder eine Kurve machen, die entsprechenden Schritte einstellen
      if (turn1)
      {
          stepper1.move(5000);
          stepper2.move(5000);
      }
      else if (turn2)
      {
          stepper1.move(turnSteps);
          stepper2.move(-turnSteps);
      }

      // Togglen der Richtung für die nächste Iteration
      if (turn1)
      {
          turn1 = false;
          turn2 = true;
      }
      else if (turn2)
      {
          turn2 = false;
      }
    }
  }


  // Motoren zu den Zielpositionen bewegen
  stepper1.runSpeedToPosition();
  stepper2.runSpeedToPosition();

  // Aktualisieren der verbleibenden Schritte
  steps1 = stepper1.distanceToGo();
  steps2 = stepper2.distanceToGo();

  Serial.print("Stepp1: ");
  Serial.println(steps1);
  Serial.print("Stepp2: ");
  Serial.println(steps2);

  
}
