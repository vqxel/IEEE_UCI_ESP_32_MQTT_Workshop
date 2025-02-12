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
const int debounceThreshold = 50;  // milliseconds to wait for a stable change
bool lastButtonState;               // last raw reading
unsigned long lastDebounceTime = 0;   // last time the raw reading changed
bool debouncedState;                // debounced (stable) state

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
  if (payload[0]) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
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
bool photoresistorReadBooleanState() {
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

  // Initialize debounce state variables with the current reading.
  lastButtonState = photoresistorReadBooleanState();
  debouncedState = lastButtonState;
  lastDebounceTime = millis();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read the current raw sensor value.
  bool reading = photoresistorReadBooleanState();
  unsigned long now = millis();

  // Check if the raw reading has changed.
  if (reading != lastButtonState) {
    // Reset the debounce timer
    lastDebounceTime = now;
  }

  // If the reading has stayed the same for longer than the debounce threshold,
  // consider it as the debounced (stable) state.
  if ((now - lastDebounceTime) > debounceThreshold) {
    if (reading != debouncedState) {
      debouncedState = reading;
      // Publish the MQTT message when the debounced state changes.
      if (debouncedState) {
        msg[0] = 1;
        Serial.println("ON");
      } else {
        msg[0] = 0;
        Serial.println("OFF");
      }
      client.publish(this_esp_id, msg, 1);
    }
  }

  // Save the current raw reading for the next loop iteration.
  lastButtonState = reading;
}