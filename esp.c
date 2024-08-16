#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi and server credentials
const char* ssid = "ssid";
const char* password = "password";
const char* loginUrl = "http://ip:3000/login";
const char* dataUrl = "http://ip:3000/data";

const char* username = "user1";
const char* userPassword = "password1";

const int ledPin = 2;  // GPIO pin for LED
const int potPin = 4;  // GPIO pin for potentiometer
const int dhtPin = 5;  // GPIO pin for DHT11 sensor

const int pwmFreq = 5000;                          // PWM frequency (5 kHz)
const int pwmResolution = 8;                       // PWM resolution (8-bit)
const int pwmMaxValue = (1 << pwmResolution) - 1;  // Max value for 8-bit resolution

const int ledChannel = 0;  // LEDC channel for PWM control


// DHT sensor setup
#define DHT_TYPE DHT11
DHT dht(dhtPin, DHT_TYPE);

String getToken() {
  HTTPClient http;
  http.begin(loginUrl);
  http.addHeader("Content-Type", "application/json");

  // Prepare JSON payload
  String payload = String("{\"username\":\"") + username + "\",\"password\":\"" + userPassword + "\"}";
  Serial.println("Sending login request");

  int httpResponseCode = http.POST(payload);
  String token = "";

  if (httpResponseCode == 200) {
    String response = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    token = doc["token"].as<String>();
  } else {
    Serial.println("Error during login: " + String(httpResponseCode));
  }

  http.end();
  return token;
}

String token;

void sendData(const String& token, const String& payload) {
  HTTPClient http;
  http.begin(dataUrl);
  http.addHeader("Authorization", "Bearer " + token);
  http.addHeader("Content-Type", "application/json");

  Serial.println("Sending data");
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Data sent successfully: " + response);
  } else {
    Serial.println("Error sending data: " + String(httpResponseCode));
  }

  http.end();
}

void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");

  if (!ledcAttach(ledPin, pwmFreq, pwmResolution)) {
    Serial.println("Failed to configure LEDC pin.");
  }

  dht.begin();
  delay(1000);

  pinMode(potPin, INPUT);

  token = getToken();
  if (token.length() > 0) {
    Serial.println("Failed to login!");
  } 
}

void loop() {
  // Read potentiometer value and map to PWM range
  int potValue = analogRead(potPin);
  int pwmValue = map(potValue, 0, 4095, 0, pwmMaxValue);

  if (!ledcWrite(ledPin, pwmValue)) {
    Serial.println("Failed to update LED brightness.");
  }

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Construct JSON payload
  String payload = "{";
  if (!isnan(temperature) && !isnan(humidity)) {
    payload += "\"temperature\": " + String(temperature) + ",";
    payload += "\"humidity\": " + String(humidity) + ",";
    payload += "\"message\": \"Operation successful!\"";
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" C\n");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\n");
  } else {
    payload += "\"message\": \"Failed to read from DHT sensor\"";
    Serial.println("Failed to read from DHT sensor. Check connections and sensor.");
    Serial.print("Temperature: NaN\n");
    Serial.print("Humidity: NaN\n");
  }
  payload += "}";

  sendData(token, payload);

  // Print LED brightness and potentiometer value
  Serial.print("Potentiometer Value: ");
  Serial.print(potValue);
  Serial.print(" -> PWM Value: ");
  Serial.println(pwmValue);

  delay(10000);
}
