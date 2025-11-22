/*
  ESP32 Network Controlled Car
  Advanced WiFi-controlled robot car with obstacle detection, GPS tracking, 
  radio communication, and modern web interface.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#define SPIFFS LittleFS

// Replace with your network credentials
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

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

// Buzzer
const int buzzerPin = 19;

// LEDs
const int led1Pin = 21;
const int led2Pin = 22;
bool led1State = false;
bool led2State = false;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

String valueString = String(0);

// APC220 Radio Module
HardwareSerial radioSerial(2);
String radioBuffer = "";
String lastRadioMessage = "No data";
unsigned long lastTelemetryTime = 0;

// NEO-M8N GPS Module
HardwareSerial gpsSerial(1);
String gpsBuffer = "";
float latitude = 0.0;
float longitude = 0.0;
int satellites = 0;
String gpsStatus = "No Fix";

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  return duration * SOUND_SPEED / 2;
}

void parseGPS() {
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    if (c == '\n') {
      if (gpsBuffer.startsWith("$GPGGA")) {
        int commaIndex[14];
        int commaCount = 0;
        for (int i = 0; i < gpsBuffer.length() && commaCount < 14; i++) {
          if (gpsBuffer.charAt(i) == ',') {
            commaIndex[commaCount++] = i;
          }
        }
        if (commaCount >= 6) {
          String latStr = gpsBuffer.substring(commaIndex[1] + 1, commaIndex[2]);
          String lonStr = gpsBuffer.substring(commaIndex[3] + 1, commaIndex[4]);
          String satStr = gpsBuffer.substring(commaIndex[6] + 1, commaIndex[7]);
          
          if (latStr.length() > 0 && lonStr.length() > 0) {
            latitude = latStr.toFloat() / 100.0;
            longitude = lonStr.toFloat() / 100.0;
            satellites = satStr.toInt();
            gpsStatus = satellites > 0 ? "Fix" : "No Fix";
          }
        }
      }
      gpsBuffer = "";
    } else {
      gpsBuffer += c;
    }
  }
}

void sendTelemetry() {
  float distance = getDistance();
  String telemetry = "DIST:" + String(distance, 1) + ",SPEED:" + valueString + ",WIFI:" + String(WiFi.RSSI()) + ",LAT:" + String(latitude, 6) + ",LON:" + String(longitude, 6);
  radioSerial.println(telemetry);
  Serial.println("Sent: " + telemetry);
}

void handleRadioData() {
  while (radioSerial.available()) {
    char c = radioSerial.read();
    if (c == '\n') {
      lastRadioMessage = radioBuffer;
      Serial.println("Received: " + lastRadioMessage);
      radioBuffer = "";
    } else {
      radioBuffer += c;
    }
  }
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

void handleRadio() {
  if (server.hasArg("msg")) {
    String message = server.arg("msg");
    radioSerial.println(message);
    Serial.println("Radio sent: " + message);
  }
  server.send(200);
}

void handleBuzzer() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "on") {
      tone(buzzerPin, 1000);
      Serial.println("Buzzer ON");
    } else {
      noTone(buzzerPin);
      Serial.println("Buzzer OFF");
    }
  }
  server.send(200);
}

void handleLED1() {
  led1State = !led1State;
  digitalWrite(led1Pin, led1State ? HIGH : LOW);
  Serial.println(led1State ? "LED1 ON" : "LED1 OFF");
  server.send(200);
}

void handleLED2() {
  led2State = !led2State;
  digitalWrite(led2Pin, led2State ? HIGH : LOW);
  Serial.println(led2State ? "LED2 ON" : "LED2 OFF");
  server.send(200);
}

void handleTelemetry() {
  float distance = getDistance();
  String json = "{\"distance\":\"" + String(distance, 1) + 
                "\",\"wifi\":\"" + String(WiFi.RSSI()) + 
                "\",\"radio\":\"" + lastRadioMessage + 
                "\",\"latitude\":\"" + String(latitude, 6) + 
                "\",\"longitude\":\"" + String(longitude, 6) + 
                "\",\"satellites\":\"" + String(satellites) + 
                "\",\"gpsStatus\":\"" + gpsStatus + "\"}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  radioSerial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  gpsSerial.begin(9600, SERIAL_8N1, 4, 2); // RX=4, TX=2

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(buzzerPin, OUTPUT); // Sets the buzzerPin as an Output
  pinMode(led1Pin, OUTPUT); // Sets LED1 as an Output
  pinMode(led2Pin, OUTPUT); // Sets LED2 as an Output

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

  // Serve static files
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");

  // Define API routes
  server.on("/forward", handleForward);
  server.on("/left", handleLeft);
  server.on("/stop", handleStop);
  server.on("/right", handleRight);
  server.on("/reverse", handleReverse);
  server.on("/speed", handleSpeed);
  server.on("/radio", handleRadio);
  server.on("/buzzer", handleBuzzer);
  server.on("/led1", handleLED1);
  server.on("/led2", handleLED2);
  server.on("/telemetry", handleTelemetry);

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
  handleRadioData();
  parseGPS();
  
  // Send telemetry every 2 seconds
  if (millis() - lastTelemetryTime > 2000) {
    sendTelemetry();
    lastTelemetryTime = millis();
  }
}