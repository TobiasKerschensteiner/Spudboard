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

AccelStepper stepper1(AccelStepper::FULL4WIRE, 18, 5, 17, 16);
AccelStepper stepper2(AccelStepper::FULL4WIRE, 26, 27, 14, 13);
MultiStepper steppers;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

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
}

void moveForward() {
  int steps = 1000;
  stepper1.move(steps);
  stepper2.move(steps);
  steppers.run();
}

void turnRight() {
  int steps = 200;
  stepper1.move(steps);
  stepper2.move(-steps);
  steppers.run();
}

void turnLeft() {
  int steps = 200;
  stepper1.move(-steps);
  stepper2.move(steps);
  steppers.run();
}

void moveBackward() {
  int steps = 1000;
  stepper1.move(-steps);
  stepper2.move(-steps);
  steppers.run();
}

void stopMotors() {
  stepper1.stop();
  stepper2.stop();
}

void loop() {
  WiFiClient client = server.available();

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

            while (forwardState == "on"){
              moveForward();
            }

            while (backwardsState == "on"){
              moveBackward();
            }

            while (leftState == "on"){
              turnLeft();
            }

            while (rightState == "on"){
              turnRight();
            }
            
            while (stopState == "on"){
              stopMotors();
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
