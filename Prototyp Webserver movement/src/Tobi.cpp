
#include <Arduino.h>

int stepCounter;
int steps = 100;

int ENABLEL = 18;
int STEPL = 5;
int DIRL = 17;

int ENABLER = 18;
int STEPR = 4;
int DIRR = 15;
int Sensor = 23;

int Wisch = 19;

void setup() 
{
  pinMode(Wisch, OUTPUT);
  pinMode(Sensor,INPUT);
  pinMode(ENABLEL, OUTPUT); // Enable
  pinMode(STEPL, OUTPUT); // Step
  pinMode(DIRL, OUTPUT); // Richtung
  digitalWrite(ENABLEL, LOW);

  Serial.begin(9600);

  pinMode(ENABLER, OUTPUT); // Enable
  pinMode(STEPR, OUTPUT); // Step
  pinMode(DIRR, OUTPUT); // Richtung
  digitalWrite(ENABLER, LOW);


  delay(4000);

}

void moveF() {
  for(stepCounter = 0; stepCounter < steps; stepCounter++) {

    digitalWrite(STEPL,HIGH);
    digitalWrite(STEPR,HIGH);
    delayMicroseconds(45);
    digitalWrite(STEPL,LOW);
    digitalWrite(STEPR,LOW);
    delayMicroseconds(45);
  }
}

void moveS() {
  for(stepCounter = 0; stepCounter < steps; stepCounter++) {

    digitalWrite(STEPL,HIGH);
    digitalWrite(STEPR,HIGH);
    delayMicroseconds(45);
    digitalWrite(STEPL,LOW);
    digitalWrite(STEPR,LOW);
    delayMicroseconds(45);
  }
}



void loop(){


    if (digitalRead(Sensor) == 0)
    {

        digitalWrite(DIRL,HIGH);
        digitalWrite(DIRR,HIGH);
        moveF();
        digitalWrite(Wisch,HIGH); 
        //Serial.println("Straight");
      
    }
    else
    {
      for (int i = 0; i <= 75; i++)
        {

            digitalWrite(DIRL,LOW);
            digitalWrite(DIRR,LOW);
            moveS();
        }
        for (int i = 0; i <= 700; i++)
        {

            digitalWrite(DIRL,HIGH);
            digitalWrite(DIRR,LOW);
            moveF();
            Serial.println("Turn");
        }

        for (int i = 0; i <= 200; i++)
        {

            digitalWrite(DIRL,HIGH);
            digitalWrite(DIRR,HIGH);
            moveS();
        }

        for (int i = 0; i <= 700; i++)
        {

            digitalWrite(DIRL,HIGH);
            digitalWrite(DIRR,LOW);
            moveS();
            Serial.println("Turn");
        }
    }
}