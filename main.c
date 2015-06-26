#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <stdio.h>
#include <sstream>
#include <string>

#define DHTTYPE DHT11
#define DHTPIN  2

unsigned long currentMillis;

char* temperatureTopic = "sensors/bedroom/temperature";
char* humidityTopic    = "sensors/bedroom/humidity";
char* heatIndexTopic   = "sensors/bedroom/heat_index";

// MQTT broker ip
char* server = "10.0.0.1";

const char* ssid     = ""; // SSID
const char* password = ""; // password

String msg;

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

DHT dht(DHTPIN, DHTTYPE, 11);

float humidity, temp_f, hi;  // Values read from sensor

// Converted string values of sensor data
char hum[10] = {0};
char heatIndex[10] = {0};
char temperature[10] = {0};

unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor

void(* resetFunc) (void) = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

void setup(void) {

  // Set baud rate
  Serial.begin(115200);

  // Initialize temperature sensor
  dht.begin();

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Generate client name based on MAC address and last 8 bits of microsecond counter
  String clientName;
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
  } else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }

}

void loop(void) {

  // Check if connected, then reconnect if not
  if (WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_DISCONNECTED){
    Serial.println("Disconnected from network, reset...");
    resetFunc();
  }

  // Get temperature and send to MQTT broker
  gettemperature();

}

void gettemperature() {
  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    // save the last time you read the sensor
    previousMillis = currentMillis;

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
    hi = dht.computeHeatIndex(temp_f, humidity);

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    floatToString(temperature, temp_f);
    floatToString(hum, humidity);
    floatToString(heatIndex, hi);

    Serial.println(temperature);
    Serial.println(hum);
    Serial.println(heatIndex);

    if (client.publish(temperatureTopic, temperature)) {
      Serial.println("Publish temperature ok");
    } else {
      Serial.println("Publish temperature failed");
    }

    if (client.publish(humidityTopic, hum)) {
      Serial.println("Publish humidity ok");
    } else {
      Serial.println("Publish humidity failed");
    }

    if (client.publish(heatIndexTopic, heatIndex)) {
      Serial.println("Publish heatIndex ok");
    } else {
      Serial.println("Publish heatIndex failed");
    }

  }

}

void floatToString(char *buf, float val) {
    String msg = "";
    msg += String(int(val)) + "." + String(getDecimal(val));
    msg.toCharArray(buf, 10);
}

String macToStr(const uint8_t* mac) {
  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5) {
      result += ':';
    }

  }

  return result;
}

long getDecimal(float val) {
  int intPart = int(val);
  long decPart = 1000 * (val - intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places.
                                      //Change to match the number of decimal places you need
  if (decPart > 0) {
    return (decPart);      //return the decimal part of float number if it is available
  } else if (decPart < 0) {
    return ((-1) * decPart); //if negative, multiply by -1
  } else if (decPart = 0) {
    return (00);           //return 0 if decimal part of float number is not available
  }

}
