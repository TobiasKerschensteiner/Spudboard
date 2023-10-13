#include <Arduino.h>
#include <Stepper.h>

//einzeln youtube video (https://www.youtube.com/watch?v=20lSgKkJVXQ)
#define Step 7
#define Dir 6

#define MS1 4
#define MS2 3
#define MS3 2

void setup()
{
    pinMode(Step, OUTPUT);
    pinMode(Dir, OUTPUT);

    pinMode(MS1, OUTPUT);
    pinMode(MS2, OUTPUT);
    pinMode(MS3, OUTPUT);
}

void loop()
{
    digitalWrite(Dir, HIGH);
    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, HIGH);
    digitalWrite(MS3, HIGH);

    digitalWrite(Step, HIGH);
    delayMicroseconds(30);
    digitalWrite(Step, LOW);
    delayMicroseconds(30);
}


//oder beispiel internet (https://funduino.de/nr-15-schrittmotor)

int SPU = 2038; // Schritte pro Umdrehung.
Stepper Motor(SPU, 3,5,4,6); // Der Schrittmotor erhält die Bezeichnung "Motor" und es wird angegeben an welchen Pins der Motor angeschlossen ist.

void setup() 
{
Motor.setSpeed(5); // Angabe der Geschwindigkeit in Umdrehungen pro Minute.
}

void loop() {
Motor.step(2038); // Der Motor macht 2048 Schritte, das entspricht einer Umdrehung.
delay(1000); 
Motor.step(-2038); // Der Motor macht durch das Minuszeichen 2048 Schritte in die andere Richtung.
delay(1000); 
}


//freestyle

// Motoren definieren
//Motor 1
#define MS11 = 3;
#define in11 = 4;
#define in22 = 5;

//Motor 2
#define MS22 = 8;
#define in33 = 7;
#define in44 = 6;

void setup()
{
    pinMode(MS11, OUTPUT);
    pinMode(MS22, OUTPUT);
    pinMode(in11, OUTPUT);
    pinMode(in22, OUTPUT);
    pinMode(in33, OUTPUT);
    pinMode(in44, OUTPUT);
}

void Forward()
{
    //Motor 1
    digitalWrite(in11, LOW);
    digitalWrite(in22, HIGH);
    
    //Motor 2
    digitalWrite(in33, LOW);
    digitalWrite(in44, HIGH);
}

void Right()
{
   //Motor 1
    digitalWrite(in11, LOW);
    digitalWrite(in22, HIGH);
    
    //Motor 2
    digitalWrite(in33, HIGH);
    digitalWrite(in44, LOW);
}

void Left()
{
    //Motor 1
    digitalWrite(in11, HIGH);
    digitalWrite(in22, LOW);
    
    //Motor 2
    digitalWrite(in33, LOW);
    digitalWrite(in44, HIGH);
}

void Backward() //falls von nöten
{
    //Motor 1
    digitalWrite(in11, HIGH);
    digitalWrite(in22, LOW);
    
    //Motor 2
    digitalWrite(in33, HIGH);
    digitalWrite(in44, LOW);
} 

void loop()
{
    Forward(); //Roboter fährt vorwärts
    delay(1000); //Dauer für größe des Kreises
    Right(); //Roboter dreht sich
    delay(500); //Dauer für die Krümmung
}