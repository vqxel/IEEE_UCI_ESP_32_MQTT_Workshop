#include <WiFi.h>
#include <PubSubClient.h>

// Pin definitions
#define LED 1
#define PHOTOTRANSISTOR 2

// Wi-Fi & MQTT configurations
const char* ssid = "Rezq24";
const char* password = "nathanmyson";
const char* mqtt_server = "test.mosquitto.org";

const char* leader_esp_id = "YOUR_NAME-esp-32";  // ID of the ESP32 who's phototransistor you want to control this ESP32's LED
const char* this_esp_id = "SOMEONE_ELSES_NAME-esp-32";    // ID of this ESP32, others will have this ID as their "leader_esp_id"

// Wi-Fi & MQTT connections
WiFiClient espClient;
PubSubClient client(espClient);
byte msg[1] = { 0 };

// Phototransistor variables
const int phototransistorThreshold = 50;

// Function to connect to Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // Turn LED on if payload is non-zero, else turn it off.

  // TODO: Read subscribed value and update the LED accordingly
  //        - payload is the array of data that's being recieved
  //        - the length will be the same as the length that's being published in loop(), in this case, 1
  //        - turn on or off the LED by looking at the value of the first index in payload
  //          - digitalWrite(LED, HIGH) and digitalWrite(LED, LOW) to turn LED on or off
  
}

// Reconnect to MQTT if disconnected
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "IEEE-mqtt-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(leader_esp_id);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Read the phototransistor state.
bool phototransistorReadBooleanState() {
  return analogRead(PHOTOTRANSISTOR) <= phototransistorThreshold;
}

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(PHOTOTRANSISTOR, INPUT);

  Serial.begin(115200);

  // Configure Wi-Fi and MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // TODO: Read sensor and update (publish) to MQTT broker
  //        - phototransistorReadBooleanState() returns whether the phototransistor is covered
  //        - msg is the byte array of data to send, change the value of its first index depending on whether the phototransistor is covered
  //        - use client.publish(this_esp_id, msg, 1); to send the message
  //          - this_esp_id is the topic to publish to
  //          - msg is the data to send
  //          - 1 is the length of the array, in this case message is 1 byte long
  bool reading = phototransistorReadBooleanState();
  
}