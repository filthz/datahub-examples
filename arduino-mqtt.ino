#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"

const char* mqtt_server = "datahub.010102.de";
const char* mqttUser = "#####";        // put your datahub username here
const char* mqttPassword = "#####";    // put your datahub password here
const char* clientId =     "#####";    // enter the device id here
const char* sensorId =     "#####";    // enter the sensor id here

// Uncomment one of the lines below for whatever DHT sensor type you're using!
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

/*Put your SSID & Password*/
const char* ssid = "#####";     // Enter your SSID here
const char* password = "####";  // Enter your wifi password here

// DHT Sensor
uint8_t DHTPin = 4;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

float Temperature;
float Humidity;

WiFiClient espClient;
PubSubClient client(espClient);

int loopCnt = 0;
int reconnectCnt = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  dht.begin();

  Serial.println("Connecting to ");
  Serial.println(ssid);

  reconnectWifi();

  client.setServer(mqtt_server, 2883);
  client.setCallback(callback);
  reconnect();
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnectWifi() {
  
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    //connect to your local wi-fi network
    WiFi.begin(ssid, password);
  
    delay(1000);
    Serial.print(".");
    reconnectCnt = reconnectCnt + 1;

    if(reconnectCnt == 1000) {
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId, mqttUser, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {   
  client.loop();
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(10000);
    return;
  }

  char tempString[8];
  char humString[8];
  String payload;
  char payloadBytes[200];
  dtostrf(t, 1, 2, tempString);
  dtostrf(h, 1, 2, humString);
  Serial.print("Temperature: ");
  Serial.println(tempString);
  Serial.print("Humidity: ");
  Serial.println(humString);

  payload = "{\"value\":";
  payload = payload + tempString;
  payload = payload + ",\"sensor_id\": \"" + sensorId + "\"}";
  payload.toCharArray(payloadBytes, 200);
  client.publish("device/" + clientId + "/rawdata", payloadBytes);

  client.loop();

  client.loop();  
  delay(10000); 

  loopCnt = loopCnt + 1;

  if(loopCnt == 60) {
    ESP.restart();
  }

  reconnectWifi();
  reconnect();
}
