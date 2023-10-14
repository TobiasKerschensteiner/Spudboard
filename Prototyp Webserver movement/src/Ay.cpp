
#include <WiFi.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

// Replace with your network credentials
const char* ssid = "Alpakhan";
const char* password = "Bananenmus";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String forwardState = "off";
String backwardsState = "off";
String leftState = "off";
String rightState = "off";
String stopState = "off"; 


// Motor Driver Pins
#define IN1 18
#define IN2 5
#define IN3 17
#define IN4 16

#define IN5 26
#define IN6 27
#define IN7 14
#define IN8 13

//pin belegung
AccelStepper stepper1(AccelStepper::FULL4WIRE, 18, 5, 17, 16);
AccelStepper stepper2(AccelStepper::FULL4WIRE, 26, 27, 14, 13);


MultiStepper steppers;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
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
}
//Vorwärtsfahren
void moveForward()
{
    int steps = 1000; // Anzahl der schritte
    stepper1.move(steps);
    stepper2.move(steps);
    steppers.run();
}

//Rechtedrehung 90°
void turnRight()
{
    int steps = 200; // Anzahl der Schritte um 90° nach rechts zu drehen
    stepper1.move(steps);
    stepper2.move(-steps);
    steppers.run();
}

//Linksdrehung 90°
void turnLeft()
{
    int steps = 200; //Anzahl der Schritte um 90° nach links zu drehen 
    stepper1.move(-steps);
    stepper2.move(steps);
    steppers.run();
}

//Rückwärtsfahren falls von nöten
void moveBackward()
{
    int steps = 1000; //Anzahl der Schritte
    stepper1.move(-steps);
    stepper2.move(-steps);
    steppers.run();
}
//Motoren stoppen
void stopMotors() {
  stepper1.stop();
  stepper2.stop();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            //if (header.indexOf("GET /26/on") >= 0) {
            if (header.indexOf("GET /1/ on")>=0){    
                //Serial.println("GPIO 26 on");
                Serial.println("Forwards on");
                forwardState = "on";
                stopState = "off";
                moveForward();
            } else if (header.indexOf("GET /1/ off") >= 0) {
              //Serial.println("GPIO 26 off");
              Serial.println("Forwards off");
              forwardState = "off";
              stopState = "on";
              stopMotors();
            //} else if (header.indexOf("GET /27/on") >= 0) {
            } else if (header.indexOf("GET /2/ on")>=0){
              //Serial.println("GPIO 27 on");
              Serial.println("Backwards on");
              backwardsState = "on";
              stopState = "off";
              moveBackward();
            //} else if (header.indexOf("GET /27/off") >= 0) {
            } else if (header.indexOf("GET /2/ off")>=0){
              //Serial.println("GPIO 27 off");
              Serial.println("Backwards off");
              backwardsState = "off";
              stopState = "on";
              stopMotors();
            } else if (header.indexOf("GET /3/ on")>=0){
                Serial.println("Left on");
                leftState = "on";
                stopState = "off";
                turnLeft();
            } else if (header.indexOf("GET /3/ off")>=0){
                Serial.println("Left off");
                leftState = "off";
                stopState = "on";
                stopMotors();
            } else if (header.indexOf("GET /4/ on")>=0){
                Serial.println("Right on");
                rightState = "on";
                stopState = "off";
                turnRight();
            } else if (header.indexOf("GET /4/ off")>=0){
                Serial.println("Right off");
                rightState = "off";
                stopState = "on";
                stopMotors();

            }    
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>PotatoWeb</h1>");
            
            // Display current state, and ON/OFF buttons for forwards  
            client.println("<p>Forward - State " + forwardState + "</p>");
            // If the Forwardstate is off, it displays the ON button       
            if (forwardState=="off") {
              client.println("<p><a href=\"Forward on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"Forward off\"><button class=\"button button2\">OFF</button></a></p>");
            } 

            // Display current state, and ON/OFF buttons for left  
            client.println("<p>Left - State " + leftState + "</p>");
            // If the BackwardsState is off, it displays the ON button       
            if (leftState=="off") {
              client.println("<p><a href=\"left on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"left off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");


            // Display current state, and ON/OFF buttons for right  
            client.println("<p>Right - State " + rightState + "</p>");
            // If the RightState is off, it displays the ON button       
            if (rightState=="off") {
              client.println("<p><a href=\"right on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"right off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
               
            // Display current state, and ON/OFF buttons for backwards  
            client.println("<p>Backwards - State " + backwardsState + "</p>");
            // If the BackwardsState is off, it displays the ON button       
            if (backwardsState=="off") {
              client.println("<p><a href=\"backwards on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"backwards off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }

   //     } else if (c != '\r') {  // if you got anything else but a carriage return character,

          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
