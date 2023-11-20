#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "MAIBOK";
const char* password = "1223334444";
const char* mqtt_server = "192.168.43.167";
const int DHTPIN = D4; // Define the pin to which the DHT sensor is connected
const int LED_PIN = D6; // Define the pin to which the LED is connected
const int DHTTYPE = DHT11; // DHT sensor type (DHT11 or DHT22)

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80); // เพิ่มเซิร์ฟเวอร์ HTTP

bool ledStatus = false;

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

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("LED"); // Subscribe to the LED topic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handleRoot() {
  String html = "<h1>Hello from ESP8266!</h1>";
  html += "<p>LED Status: " + String(ledStatus ? "ON" : "OFF") + "</p>";
  html += "<form action='/led/on' method='get'><input type='submit' value='Turn On'></form>";
  html += "<form action='/led/off' method='get'><input type='submit' value='Turn Off'></form>";
  server.send(200, "text/html", html);
}

void handleLedOn() {
  client.publish("LED", "ON");
  digitalWrite(LED_PIN, HIGH);
  ledStatus = true;
  handleRoot();
}

void handleLedOff() {
  client.publish("LED", "OFF");
  digitalWrite(LED_PIN, LOW);
  ledStatus = false;
  handleRoot();
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  dht.begin();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/led/on", HTTP_GET, handleLedOn);
  server.on("/led/off", HTTP_GET, handleLedOff);

  server.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  // Get the IP Address of the device
  String ip_address = WiFi.localIP().toString();

  // Create a JSON document
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["IP Address"] = ip_address;

  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);

  // Publish the JSON payload to the "dht11" topic
  client.publish("dht11", jsonStr.c_str());

  delay(5000);
  client.loop();
  server.handleClient();
}
