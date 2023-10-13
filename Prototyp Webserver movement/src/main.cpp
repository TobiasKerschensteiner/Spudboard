#include <Arduino.h>
#include <WiFi.h>
#include <AccelStepper.h>
#include <AsyncTCP.h>

// Replace with your network credentials
const char* ssid = "Alpakhan";
const char* password = "Bananenmus";

// Set web server port number to 80
WiFiServer server(80);

// Motor Driver Pins
#define IN1 18
#define IN2 5
#define IN3 17
#define IN4 16

#define IN5 26
#define IN6 27
#define IN7 14
#define IN8 13

// Define the number of steps per revolution
const int stepsPerRevolution = 2048;

// Initialize the stepper library
AccelStepper stepper1(AccelStepper::FULL4WIRE, IN1, IN2, IN3, IN4);
AccelStepper stepper2(AccelStepper::FULL4WIRE, IN5, IN6, IN7, IN8);

// Motor speeds for Forward and Backward movement
const int motorSpeedForward = 1000; // Adjust as needed
const int motorSpeedBackward = -1000; // Adjust as needed

// HTML page as a C++ string
const char* htmlPage = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <title>ESP32-CAM Robot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-Left: auto; margin-Right: auto; }
      td { padding: 8 px; }
      .button {
        background-color: #2f4468;
        border: none;
        color: white;
        padding: 10px 20px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }
      img {  width: auto ;
        max-width: 100% ;
        height: auto ; 
      }
    </style>
  </head>
  <body>
    <h1>Wischbot</h1>
    <img src="" id="photo" >
    <table>
      <tr><td colspan="3" align="center"><button class="button" onmousedown="toggleCheckbox('Forward');" ontouchstart="toggleCheckbox('Forward');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Forward</button></td></tr>
      <tr><td align="center"><button class="button" onmousedown="toggleCheckbox('Left');" ontouchstart="toggleCheckbox('Left');" onmouseup="toggleCheckbox('Stop');" ontouchend="toggleCheckbox('stop');">Left</button></td><td align="center"><button class="button" onmousedown="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('stop');">Stop</button></td><td align="center"><button class="button" onmousedown="toggleCheckbox('Right');" ontouchstart="toggleCheckbox('Right');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Right</button></td></tr>
      <tr><td colspan="3" align="center"><button class="button" onmousedown="toggleCheckbox('Backward');" ontouchstart="toggleCheckbox('Backward');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Backward</button></td></tr>                   
    </table>
   <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/action?go=" + x, true);
     xhr.send();
   }
   window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
  </script>
  </body>
</html>
)=====";

void setup() {
  Serial.begin(9600);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  // Set the initial speed and acceleration of the motors
  stepper1.setMaxSpeed(1000); // Adjust as needed
  stepper2.setMaxSpeed(1000); // Adjust as needed
  stepper1.setAcceleration(1000); // Adjust as needed
  stepper2.setAcceleration(1000); // Adjust as needed
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.print(htmlPage); // Send the HTML page

            if (currentLine.indexOf("Forward") != -1) {
              Serial.println("Forward");
              stepper1.setSpeed(motorSpeedForward);
              stepper2.setSpeed(motorSpeedForward);
            } else if (currentLine.indexOf("Backward") != -1) {
              Serial.println("Backward");
              stepper1.setSpeed(motorSpeedBackward);
              stepper2.setSpeed(motorSpeedBackward);
            } else if (currentLine.indexOf("Left") != -1) {
              Serial.println("Left");
              stepper1.setSpeed(motorSpeedBackward);
              stepper2.setSpeed(motorSpeedForward);
            } else if (currentLine.indexOf("Right") != -1) {
              Serial.println("Right");
              stepper1.setSpeed(motorSpeedForward);
              stepper2.setSpeed(motorSpeedBackward);
            } else if (currentLine.indexOf("Stop") != -1) {
              Serial.println("Stop");
              stepper1.setSpeed(0);
              stepper2.setSpeed(0);
            }

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
