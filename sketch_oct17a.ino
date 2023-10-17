#include <NewPing.h>       // Include the NewPing library
#include <Arduino_LSM6DS3.h>
#include <WiFiNINA.h>
#define SECRET_SSID "Aadarsh"
#define SECRET_PASSWORD "123456789"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

// Constants for pin numbers
const int TRIG_PIN = 7;
const int ECHO_PIN = 6;
const int BUZZER_PIN = 8;

// Constants for sensor limits
const int MAX_DISTANCE = 200;
const int BUZZER_THRESHOLD = 35; // Adjust the threshold

// Create a NewPing object
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Constants for fall detection thresholds
const float GyroThreshold = 150.0;
const int DetectionDelay = 5000;

bool fallDetected = false;
unsigned long lastDetectionTime = 0;


const char* iftttEmailEvent = "Fall_Detected";
const char* iftttSMSEvent = "Fall_SMS";
const char* iftttKey = "lw9kqxc0zygN358R7a6AnIA4GuIHbCHylyu3g26yXMJ";  

WiFiClient client;  // Reuse the client for HTTP requests

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
  
  Serial.begin(9600);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Measure distance
  unsigned int distance = sonar.ping_cm();

  // Print distance
  //Serial.println("Distance: ");
  Serial.println(distance);
  //Serial.println(" cm");

  // Check distance and control buzzer
  if (distance <= BUZZER_THRESHOLD && distance > 0) {
    // Set the buzzer to its maximum intensity
    analogWrite(BUZZER_PIN, 255);
  } else {
    // Turn off the buzzer when distance is outside the threshold
    analogWrite(BUZZER_PIN, 0);
  }

  float x, y, z;

  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(x, y, z);

    // Calculate the magnitude of angular velocity
    float gyroMagnitude = sqrt(x * x + y * y + z * z);

    // Check if the gyroscope magnitude exceeds the threshold
    if (gyroMagnitude > GyroThreshold) {
      // Check if enough time has passed since the last detection to avoid false positives
      unsigned long currentTime = millis();
      if (currentTime - lastDetectionTime > DetectionDelay) {
        // Fall detected
        Serial.println("Fall detected!");
        fallDetected = true;
        lastDetectionTime = currentTime;

        // Trigger both IFTTT events
        sendIFTTTEvent(iftttEmailEvent);
        sendIFTTTEvent(iftttSMSEvent);
      }
    }
  }
}

void sendIFTTTEvent(const char* event) {
  // Construct the IFTTT Webhooks URL
  String url = "/trigger/" + String(event) + "/with/key/" + iftttKey;
  
  // Prepare the POST data
  String postData = "value1=Fall Detected";

  // Send an HTTP POST request to trigger the IFTTT event
  if (client.connect("maker.ifttt.com", 80)) {
    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println("Connection: close");
    client.println();
    client.println(postData);
    delay(10);
    client.stop();
  }
}
