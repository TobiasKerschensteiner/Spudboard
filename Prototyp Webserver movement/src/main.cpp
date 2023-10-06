#include <Arduino.h>
#include <WiFi.h>
#include <AccelStepper.h>

// Replace with your network credentials
const char* ssid = "WLAN";
const char* password = "Password";

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

// Motor speeds for forward and backward movement
const int motorSpeedForward = 1000; // Adjust as needed
const int motorSpeedBackward = -1000; // Adjust as needed

// HTML page as a C++ string
const char* htmlPage = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        html {
            font-family: Helvetica;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }
        .button {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 16px 40px;
            text-decoration: none;
            font-size: 30px;
            margin: 2px;
            cursor: pointer;
        }
        .button2 {
            background-color: #555555;
        }
        .controls {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            grid-template-rows: repeat(3, 1fr);
            gap: 10px;
            align-items: center;
            justify-content: center;
            height: 300px;
        }
        .arrow-button {
            background-color: #3498db;
            border: none;
            color: white;
            padding: 16px 40px;
            text-decoration: none;
            font-size: 30px;
            margin: 2px;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <h1>Remote Control Car</h1>
    <div class="controls">
        <a href="/Forward"><button class="arrow-button" style="grid-column: 2; grid-row: 1;" onclick="sendCommand('/Forward');">&#8593;</button></a>
        <a href="/Left"><button class="arrow-button" style="grid-column: 1; grid-row: 2;" onclick="sendCommand('/Left');">&#8592;</button></a>
        <a href="/Right"><button class="arrow-button" style="grid-column: 3; grid-row: 2;" onclick="sendCommand('/Right');">&#8594;</button></a>
        <a href="/Stop"><button class="arrow-button" style="grid-column: 2; grid-row: 2; background-color: #e74c3c;" onclick="sendCommand('/Stop');">■</button></a>
        <a href="/Backward"><button class="arrow-button" style="grid-column: 2; grid-row: 3;" onclick="sendCommand('/Backward');">&#8595;</button></a>
    </div>

    <script>
        function sendCommand(command) {
            // Send an HTTP request to the ESP32 with the selected command
            var xhr = new XMLHttpRequest();
            xhr.open("GET", command, true);
            xhr.send();
        }
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
