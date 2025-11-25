/*
  ESP32 Network Controlled Car
  Advanced WiFi-controlled robot car with dual obstacle detection (ultrasonic + IR), 
  GPS tracking, and modern web interface.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#define SPIFFS LittleFS

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

// HW-870 IR Obstacle Sensor (Analog)
const int irSensorPin = 17;
int irValue = 0;
bool irDetected = false;

// Master obstacle detection control
bool obstacleDetectionEnabled = true;

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

// Cache distance reading to avoid blocking
float lastDistance = 100.0;
unsigned long lastDistanceTime = 0;

bool checkForwardObstacle() {
  if (!obstacleDetectionEnabled) return false;
  
  // Only read distance every 100ms to avoid blocking
  if (millis() - lastDistanceTime > 100) {
    lastDistance = getDistance();
    lastDistanceTime = millis();
  }
  
  return lastDistance <= 10.0; // Obstacle detected if distance <= 10cm
}

// Cache IR reading
int lastIrValue = 0;
unsigned long lastIrTime = 0;

bool checkBackwardObstacle() {
  if (!obstacleDetectionEnabled) return false;
  
  // Only read IR every 50ms
  if (millis() - lastIrTime > 50) {
    lastIrValue = analogRead(irSensorPin);
    lastIrTime = millis();
  }
  
  irValue = lastIrValue;
  irDetected = irValue > 2000; // Adjust threshold as needed
  return irDetected;
}



void handleForward() {
  if (checkForwardObstacle()) {
    Serial.println("Ultrasonic obstacle detected - Forward blocked");
    handleStop();
  } else {
    Serial.println("Forward");
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH); 
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, HIGH);
  }
  server.send(200);
}

void handleLeft() {
  Serial.println("Left");
  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, LOW); 
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
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
  Serial.println("Right");
  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, HIGH); 
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  server.send(200);
}

void handleReverse() {
  if (checkBackwardObstacle()) {
    Serial.println("IR obstacle detected - Reverse blocked");
    handleStop();
  } else {
    Serial.println("Reverse");
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW); 
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
  }
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

void handleObstacleToggle() {
  obstacleDetectionEnabled = !obstacleDetectionEnabled;
  Serial.println(obstacleDetectionEnabled ? "Obstacle detection ENABLED" : "Obstacle detection DISABLED");
  server.send(200);
}

void handleTelemetry() {
  // Use cached values to avoid blocking
  bool forwardBlocked = checkForwardObstacle();
  bool backwardBlocked = checkBackwardObstacle();
  
  String json = "{\"distance\":\"" + String(lastDistance, 1) + 
                "\",\"wifi\":\"" + String(WiFi.RSSI()) + 
                "\",\"irValue\":" + String(lastIrValue) + 
                ",\"irDetected\":" + (irDetected ? "true" : "false") + 
                ",\"forwardBlocked\":" + (forwardBlocked ? "true" : "false") + 
                ",\"backwardBlocked\":" + (backwardBlocked ? "true" : "false") + 
                ",\"obstacleDetectionEnabled\":" + (obstacleDetectionEnabled ? "true" : "false") + 
                ",\"motorStatus\":\"Ready\"" +
                ",\"latitude\":\"" + String(latitude, 6) + 
                "\",\"longitude\":\"" + String(longitude, 6) + 
                "\",\"satellites\":\"" + String(satellites) + 
                "\",\"gpsStatus\":\"" + gpsStatus + "\"}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 4, 2); // RX=4, TX=2

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  // HW-870 IR sensor is analog, no pinMode needed for analog pins
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
    
  // Initialize PWM with default duty cycle
  ledcWrite(enable1Pin, 200);
  ledcWrite(enable2Pin, 200);
  
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

  server.on("/buzzer", handleBuzzer);
  server.on("/led1", handleLED1);
  server.on("/led2", handleLED2);
  server.on("/obstacle-toggle", handleObstacleToggle);
  server.on("/telemetry", handleTelemetry);

  // Start the server
  server.begin();
}

void loop() {
  server.handleClient();
  
  // Parse GPS less frequently to reduce blocking
  static unsigned long lastGpsTime = 0;
  if (millis() - lastGpsTime > 200) {
    parseGPS();
    lastGpsTime = millis();
  }
  
  // Update sensor readings in background
  checkForwardObstacle();
  checkBackwardObstacle();
}