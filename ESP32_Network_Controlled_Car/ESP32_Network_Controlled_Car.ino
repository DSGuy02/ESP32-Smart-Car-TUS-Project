
/*
#include "BluetoothSerial.h"


BluetoothSerial SerialBT;


int Motor1Pin1 = 26;
int Motor1Pin2 = 27;

int Motor2Pin1 = 25;
int Motor2Pin2 = 33;

String SBT;

bool hasLooped = false;

void setup() {
   Serial.begin(115200);
   SerialBT.begin("ESP32_Car");
   
    
   pinMode(Motor1Pin1, OUTPUT);
   pinMode(Motor1Pin2, OUTPUT);
   pinMode(Motor2Pin1, OUTPUT);
   pinMode(Motor2Pin2, OUTPUT);
}


void loop() {

   if (!hasLooped) {
      Serial.println("Starting");
      digitalWrite(Motor1Pin1, HIGH);
      digitalWrite(Motor1Pin2, LOW);
      digitalWrite(Motor2Pin1, HIGH);
      digitalWrite(Motor2Pin2, LOW);
      
      delay(2000);

      digitalWrite(Motor1Pin1, LOW);
      digitalWrite(Motor1Pin2, HIGH);
      digitalWrite(Motor2Pin1, LOW);
      digitalWrite(Motor2Pin2, HIGH);

      delay(2000);
      
      Serial.println("Finished");
   }

   
   hasLooped = true;
   digitalWrite(Motor1Pin1, LOW);
   digitalWrite(Motor1Pin2, LOW);
   digitalWrite(Motor2Pin1, LOW);
   digitalWrite(Motor2Pin2, LOW);

}

void looped() {
   if (Serial.available()) {
      SerialBT.write(Serial.read());
   }

   if (SerialBT.available()) {
      Serial.write(SerialBT.read());
   }

   delay(5);
   SBT = SerialBT.readString();
   Serial.println(SBT);

   if (SBT == "f") {
      //digitalWrite(C1, HIGH);
      //digitalWrite(A1, LOW);
      //digitalWrite(C2, HIGH);
      //digitalWrite(A2, LOW);
   }

   else if (SBT == "b") {
      //digitalWrite(C1, LOW);
      //digitalWrite(A1, HIGH);
      //digitalWrite(C2, LOW);
      //digitalWrite(A2, HIGH);
   }

   else if (SBT == "l") {
      //digitalWrite(C1, LOW);
      //digitalWrite(A1, HIGH);
      //digitalWrite(C2, HIGH);
      //digitalWrite(A2, LOW);
      delay(200);
    
      //digitalWrite(C1, LOW);
      //digitalWrite(A1, LOW);
      //digitalWrite(C2, LOW);
      //digitalWrite(A2, LOW);
   }

   else if (SBT == "r") {
      //digitalWrite(C1, HIGH);
      //digitalWrite(A1, LOW);
      //digitalWrite(C2, LOW);
      //digitalWrite(A2, HIGH);
      
      delay(200);
      
      //digitalWrite(C1, LOW);
      //digitalWrite(A1, LOW);
      //digitalWrite(C2, LOW);
      //digitalWrite(A2, LOW); 
   } else {
      //digitalWrite(C1, LOW);
      //digitalWrite(A1, LOW);
      //digitalWrite(C2, LOW);
      //digitalWrite(A2, LOW); 
   }
}

*/


/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/esp32-wi-fi-car-robot-arduino/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/


#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid     = "D.S CE 3";
const char* password = "01ad3j!5";

// Create an instance of the WebServer on port 80
WebServer server(80);

// Motor 1
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;

// Motor 2
int motor2Pin1 = 33; 
int motor2Pin2 = 25; 
int enable2Pin = 32;

// Setting PWM properties
const int freq = 30000;
const int resolution = 8;
int dutyCycle = 0;

// Variables for Ultrasponic
const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

String valueString = String(0);

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  return duration * SOUND_SPEED / 2;
}

void handleRoot() {
  const char html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
      html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }
      .button { -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; background-color: #4CAF50; border: none; color: white; padding: 12px 28px; text-decoration: none; font-size: 26px; margin: 1px; cursor: pointer; }
      .button2 {background-color: #555555;}
    </style>
    <script>
      function moveForward() { fetch('/forward'); }
      function moveLeft() { fetch('/left'); }
      function stopRobot() { fetch('/stop'); }
      function moveRight() { fetch('/right'); }
      function moveReverse() { fetch('/reverse'); }

      function updateMotorSpeed(pos) {
        document.getElementById('motorSpeed').innerHTML = pos;
        fetch(`/speed?value=${pos}`);
      }
    </script>
  </head>
  <body>
    <h1>ESP32 Motor Control</h1>
    <p><button class="button" onclick="moveForward()">FORWARD</button></p>
    <div style="clear: both;">
      <p>
        <button class="button" onclick="moveLeft()">LEFT</button>
        <button class="button button2" onclick="stopRobot()">STOP</button>
        <button class="button" onclick="moveRight()">RIGHT</button>
      </p>
    </div>
    <p><button class="button" onclick="moveReverse()">REVERSE</button></p>
    <p>Motor Speed: <span id="motorSpeed">0</span></p>
    <input type="range" min="0" max="100" step="25" id="motorSlider" oninput="updateMotorSpeed(this.value)" value="0"/>
  </body>
  </html>)rawliteral";
  server.send(200, "text/html", html);
}

void handleForward() {
  float distance = getDistance();
  if (distance > 10) {
    Serial.println("Forward");
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH); 
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
  } else {
    Serial.println("Obstacle detected - Forward blocked");
    handleStop();
  }
  server.send(200);
}

void handleLeft() {
  float distance = getDistance();
  if (distance > 10) {
    Serial.println("Left");
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW); 
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
  } else {
    Serial.println("Obstacle detected - Left blocked");
    handleStop();
  }
  server.send(200);
}

void handleStop() {
  Serial.println("Stop");
  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, LOW); 
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);   
  server.send(200);
}

void handleRight() {
  float distance = getDistance();
  if (distance > 10) {
    Serial.println("Right");
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH); 
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);
  } else {
    Serial.println("Obstacle detected - Right blocked");
    handleStop();
  }
  server.send(200);
}

void handleReverse() {
  Serial.println("Reverse");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW); 
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);          
  server.send(200);
}

void handleSpeed() {
  if (server.hasArg("value")) {
    valueString = server.arg("value");
    int value = valueString.toInt();
    if (value == 0) {
      ledcWrite(enable1Pin, 0);
      ledcWrite(enable2Pin, 0);
      digitalWrite(motor1Pin1, LOW); 
      digitalWrite(motor1Pin2, LOW); 
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, LOW);   
    } else { 
      dutyCycle = map(value, 25, 100, 200, 255);
      ledcWrite(enable1Pin, dutyCycle);
      ledcWrite(enable2Pin, dutyCycle);
      Serial.println("Motor speed set to " + String(value));
    }
  }
  server.send(200);
}



void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  // Set the Motor pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  // Configure PWM Pins
  ledcAttach(enable1Pin, freq, resolution);
  ledcAttach(enable2Pin, freq, resolution);
    
  // Initialize PWM with 0 duty cycle
  ledcWrite(enable1Pin, 100);
  ledcWrite(enable2Pin, 100);
  
  // Connect to Wi-Fi
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

  // Define routes
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/left", handleLeft);
  server.on("/stop", handleStop);
  server.on("/right", handleRight);
  server.on("/reverse", handleReverse);
  server.on("/speed", handleSpeed);

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
}
  //delay(2000);
}


