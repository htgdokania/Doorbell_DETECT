#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>


// WiFi credentials
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";
const char* url1 = "VIRTUAL_BUTTON_URL";

// Doorbell pin
const int doorbellPin = D1;
WiFiClient client1;

// Adafruit.io credentials
#define AIO_USERNAME "ADAFRUIT_USER_ID"
#define AIO_KEY "ADAFRUIT_API_KEY"

Adafruit_MQTT_Client mqttClient(&client1, "io.adafruit.com", 1883, AIO_USERNAME, AIO_KEY);

// MQTT topics
Adafruit_MQTT_Publish doorbellState = Adafruit_MQTT_Publish(&mqttClient, AIO_USERNAME "/feeds/doorbell-state");

// Web server
ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  pinMode(doorbellPin, INPUT);
  pinMode(D1, OUTPUT);

  // Connect to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to Adafruit.io MQTT broker
  connectToMqttBroker();

  // Start web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started");
  digitalWrite(doorbellPin,HIGH);
}

void loop() {
  server.handleClient();

  // Check if doorbell is pressed
  if (digitalRead(doorbellPin) == LOW) {
    Serial.println("Doorbell pressed");
    //triggerAlexaRoutine(url1);
    //triggerAlexaRoutine(url2);
    
    triggerAlexaRoutine2();    
    publishDoorbellState("pressed");
    delay(5000); // wait 5 seconds to avoid sending multiple notifications
    digitalWrite(doorbellPin,HIGH);
  }
  
}

void handleRoot() {
  server.send(200, "text/html", "<h1>Doorbell automation</h1><p>Doorbell automation is running</p>");
}

void connectToMqttBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to Adafruit.io MQTT broker...");
    if (mqttClient.connect()) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      //Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void publishDoorbellState(const char* state) {
  if (mqttClient.connected()) {
    doorbellState.publish(state);
    Serial.println("Doorbell state published");
  } else {
    Serial.println("Failed to publish doorbell state: not connected to MQTT broker");
    connectToMqttBroker();
  }
}

void triggerAlexaRoutine(const char* url){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    
    // Ignore SSL certificate validation
    client->setInsecure();
    
    //create an HTTPClient instance
    HTTPClient https;
    
    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, url)) {  // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  
  
}

void triggerAlexaRoutine2(){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    
    // Ignore SSL certificate validation
    client->setInsecure();
    
    //create an HTTPClient instance
    HTTPClient https;
    
    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    https.begin(*client, url1);
    https.GET();
    https.begin(*client, url2);
    https.GET();  
    https.end();
}
