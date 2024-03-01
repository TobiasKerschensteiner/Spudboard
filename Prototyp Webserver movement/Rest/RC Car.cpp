#include <WiFi.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

const char* ssid = "Alpakhan";
const char* password = "Bananenmus";
bool motorsRunning = false;

WiFiServer server(80);

String header;
String forwardState = "off";
String backwardsState = "off";
String leftState = "off";
String rightState = "off";
String stopState = "off";

#define FULLSTEP 4
#define HALFSTEP 8
//pin belegung
AccelStepper stepper1(HALFSTEP, 17, 18, 5, 16);  
AccelStepper stepper2(HALFSTEP, 26, 14, 27, 12);
const short fullRevolution = 2048;
const float SteppDegree = 11.32;
int maxstepperSpeed = 2000;
int stepperSpeed = 200;
int stepperBesch = 100;
int sensor = 15;


boolean turn1 = true; //track ob wir gerade fahren oder in einer Kurve
boolean turn2 = true;

MultiStepper steppers;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;


int stepCounter;
int steps = 1000;
int ENABLEL = 18;
int STEPL = 4;
int DIRL = 17;

int ENABLER = 18;
int STEPR = 4;
int DIRR = 15;

String htmlTemplate = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "<link rel=\"icon\" href=\"data:,\">"
  "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
  ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
  ".button2 {background-color: #555555;}</style></head>";

void setup() {
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
    // Max Geschwindigkeit soll nicht mehr als 2000 betragen
  // stepper1.setMaxSpeed(200);
  // stepper2.setMaxSpeed(200);

  // //sanfter anlauf
  // stepper1.setAcceleration(100);
  // stepper2.setAcceleration(100);

  // //Stepper zusammen fügen. Geht bis zu Max 10
  // steppers.addStepper(stepper1);
  // steppers.addStepper(stepper2);
  delay(3000);

  stepper1.setMaxSpeed(maxstepperSpeed);
  stepper1.move(1);
  stepper1.setAcceleration(stepperBesch);
  stepper1.setSpeed(stepperSpeed);
  stepper1.setCurrentPosition(0);

  stepper2.setMaxSpeed(maxstepperSpeed);
  stepper2.move(-1);
  stepper2.setAcceleration(stepperBesch);
  stepper2.setSpeed(stepperSpeed);
  stepper2.setCurrentPosition(0);
}

void turnRight()
{
  float degree = 180;
  float moveRev = degree * SteppDegree;
  stepper1.moveTo(moveRev);
  stepper1.run();
  stepper2.moveTo(-moveRev);
  stepper2.run();
}

void turnLeft()
{
  float degree = 180;
  float moveRev = degree * SteppDegree;
  stepper1.moveTo(-moveRev);
  stepper1.run();
  stepper2.moveTo(moveRev);
  stepper2.run();
}

void moveForward()
{
  stepper1.moveTo(100);
  stepper1.run();
  stepper2.moveTo(100);
  stepper2.run();
}

void stopMovement()
{
  stepper1.stop();
  stepper2.stop();
}
//hhjj
void moveBackward()
{
  stepper1.moveTo(-100);
  stepper1.run();
  stepper2.moveTo(-100);
  stepper2.run();
}




// //Vorwärtsfahren
// void moveForward() {
//   digitalWrite(DIRL, HIGH); // im Uhrzeigersinn
//   digitalWrite(DIRR, HIGH); // im Uhrzeigersinn  
//   for(stepCounter = 0; stepCounter < steps; stepCounter++) {
//     digitalWrite(STEPL, HIGH);
//     digitalWrite(STEPR, HIGH);
//     delayMicroseconds(70);
//     digitalWrite(STEPL, LOW);
//     digitalWrite(STEPR, LOW);
//     delayMicroseconds(70);
//   }
// }

// //Rechtedrehung 90°
// void turnRight() {
//   digitalWrite(DIRL, HIGH); // im Uhrzeigersinn
//   digitalWrite(DIRR, LOW); // gegen den Uhrzeigersinn  
//   for(stepCounter = 0; stepCounter < steps; stepCounter++) {
//     digitalWrite(STEPL, HIGH);
//     digitalWrite(STEPR, HIGH);
//     delayMicroseconds(70);
//     digitalWrite(STEPL, LOW);
//     digitalWrite(STEPR, LOW);
//     delayMicroseconds(70);
//   }
// }

// //Linksdrehung 90°
// void turnLeft() {
//   digitalWrite(DIRL, LOW); // gegen den Uhrzeigersinn
//   digitalWrite(DIRR, HIGH); // im Uhrzeigersinn  
//   for(stepCounter = 0; stepCounter < steps; stepCounter++) {
//     digitalWrite(STEPL, HIGH);
//     digitalWrite(STEPR, HIGH);
//     delayMicroseconds(70);
//     digitalWrite(STEPL, LOW);
//     digitalWrite(STEPR, LOW);
//     delayMicroseconds(70);
//   }
// }

// //Rückwärtsfahren falls von nöten
// void moveBackward() {
//   digitalWrite(DIRL, LOW); // gegen den Uhrzeigersinn
//   digitalWrite(DIRR, LOW); // gegen den Uhrzeigersinn  
//   for(stepCounter = 0; stepCounter < steps; stepCounter++) {
//     digitalWrite(STEPL, HIGH);
//     digitalWrite(STEPR, HIGH);
//     delayMicroseconds(70);
//     digitalWrite(STEPL, LOW);
//     digitalWrite(STEPR, LOW);
//     delayMicroseconds(70);
//   }
// }
// void stopMovement() {
//   digitalWrite(STEPL, LOW);
//   digitalWrite(STEPR, LOW);
// }

void loop() {
  while (1)
{  WiFiClient client = server.available();

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

            if (header.indexOf("/1/on") >= 0) {
              Serial.println("Forwards on");
              forwardState = "on";
              stopState = "off";
              
            } else if (header.indexOf("/1/off") >= 0) {
              Serial.println("Forwards off");
              forwardState = "off";
              stopState = "on";
            } else if (header.indexOf("/2/on") >= 0) {
              Serial.println("Backwards on");
              backwardsState = "on";
              stopState = "off";
            } else if (header.indexOf("/2/off") >= 0) {
              Serial.println("Backwards off");
              backwardsState = "off";
              stopState = "on";
            } else if (header.indexOf("/3/on") >= 0) {
              Serial.println("Left on");
              leftState = "on";
              stopState = "off";
            } else if (header.indexOf("/3/off") >= 0) {
              Serial.println("Left off");
              leftState = "off";
              stopState = "on";
            } else if (header.indexOf("/4/on") >= 0) {
              Serial.println("Right on");
              rightState = "on";
              stopState = "off";
            } else if (header.indexOf("/4/off") >= 0) {
              Serial.println("Right off");
              rightState = "off";
              stopState = "on";
            }

            if (forwardState == "on"){
              moveForward();
            Serial.print("Vor");
            }

            if (backwardsState == "on"){
              moveBackward();
              Serial.print("Rückwärts");
            }

            if (leftState == "on"){
              turnLeft();
            Serial.print("Links");              
            }

            if (rightState == "on"){ 
              turnRight();
            Serial.print("Rechts");
            }
            
            if (stopState == "on"){
              stopMovement();
            Serial.print("Stopp");
            }

            String response = htmlTemplate;
            response += "<body><h1>PotatoWeb</h1>";
            response += "<p>Forward - State " + forwardState + "</p>";

            if (backwardsState == "off" && forwardState == "off" && leftState == "off" && rightState == "off") {

              response += "<p><a href=\"/1/on\"><button class=\"button\">ON</button></a></p>";
            } else if (forwardState == "on") {
              response += "<p><a href=\"/1/off\"><button class=\"button button2\">OFF</button></a></p>";
            }

            response += "<p>Backwards - State " + backwardsState + "</p>";

            if (backwardsState == "off" && forwardState == "off" && leftState == "off" && rightState == "off") {

              response += "<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>";
            } else if(backwardsState == "on") {
              response += "<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>";
            }

            response += "<p>Left - State " + leftState + "</p>";

            if (backwardsState == "off" && forwardState == "off" && leftState == "off" && rightState == "off") {

              response += "<p><a href=\"/3/on\"><button class=\"button\">ON</button></a></p>";
            } else if(leftState == "on") {
              response += "<p><a href=\"/3/off\"><button class=\"button button2\">OFF</button></a></p>";
            }

            response += "<p>Right - State " + rightState + "</p>";

            if (backwardsState == "off" && forwardState == "off" && leftState == "off" && rightState == "off") {

              response += "<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>";
            } else if(rightState == "on") {
              response += "<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>";
            }

            response += "</body></html>";
            client.println(response);
            break;
          } else {
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
}
}