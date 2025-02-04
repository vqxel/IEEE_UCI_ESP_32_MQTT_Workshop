#include <WiFi.h>
#include <PubSubClient.h>

// Pin definitions
#define LED 1
#define BUTTON 2

// Wi-Fi & MQTT configurations
const char* ssid = "Rezq24";
const char* password = "nathanmyson";
const char* mqtt_server = "test.mosquitto.org";

const char* leader_esp_id = "SOMEONE_ELSES_NAME-esp-32"; // ID of the ESP32 who's button you want to control this ESP32's LED, someone elses "this_esp_id"
const char* this_esp_id = "YOUR_NAME-esp-32"; // ID of this ESP32, other's will have this ID as their "leader_esp_id"

// Wi-Fi & MQTT connections
WiFiClient espClient;
PubSubClient client(espClient);
byte msg[1] = {0};

// Debounce variables
const int debounceThreshold = 100;
bool lastButtonState;
unsigned long lastButtonPress;
unsigned long lastButtonDepress;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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

// This function will get called when a topic this ESP is subscribed to updates
void callback(char* topic, byte* payload, unsigned int length) {
  // Switch on the LED if the payload is a 1, otherwise turn it off
  if (payload[0]) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "IEEE-mqtt-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(leader_esp_id);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // Configure IO
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT);

  Serial.begin(115200);

  // Configure Wi-Fi
  setup_wifi();

  // Configure MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Initialize state variables
  lastButtonState = digitalRead(BUTTON);
  lastButtonPress = 0;
  lastButtonDepress = 0;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Toggle follwer LEDs
  bool buttonState = digitalRead(BUTTON);
  unsigned long now = millis();
  if (buttonState) {
    // Debounce button press
    if (!lastButtonState && (now - lastButtonPress) > debounceThreshold) {
      // Send message to turn on LED
      msg[0] = 1;
      client.publish(this_esp_id, msg, 1);
    }
    lastButtonPress = now;
  } else {
    // Debounce button depress
    if (lastButtonState && (now - lastButtonDepress) > debounceThreshold) {
      // Send message to turn off LED
      msg[0] = 0;
      client.publish(this_esp_id, msg, 1);
    }
    lastButtonDepress = now;
  }
  lastButtonState = buttonState;
}