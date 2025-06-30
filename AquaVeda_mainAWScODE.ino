#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* awsApiEndpoint = "https://YOUR_API_GATEWAY_ID.execute-api.YOUR_REGION.amazonaws.com/YOUR_STAGE/";

const int GSR_PIN = 36;

#define DHT_PIN 17
#define DHT_TYPE DHT21
DHT dht(DHT_PIN, DHT_TYPE);

const int ONE_WIRE_BUS = 16;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int LED_R_PIN = 27;
const int LED_G_PIN = 26;
const int LED_B_PIN = 25;

const int GSR_THRESHOLD_ALERT = 2500;
const int GSR_THRESHOLD_MODERATE = 1500;
const int GSR_THRESHOLD_NORMAL = 500;

const long gmtOffset_sec = 5 * 3600 + 30 * 60;
const int daylightOffset_sec = 0;
const char* ntpServer = "pool.ntp.org";

unsigned long lastSendTime = 0;
const long sendInterval = 5000;

void connectWiFi();
void initNTP();
long getUnixTimestamp();
void setRGBColor(int red, int green, int blue);
void sendDataToAWS(float gsrVoltage, float dhtTemp, float dhtHum, float dsTemp, long timestamp);

void setup() {
  Serial.begin(115200);

  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  setRGBColor(0, 0, 0);

  Serial.println("Initializing DHT sensor...");
  dht.begin();

  Serial.println("Initializing DS18B20 sensor...");
  sensors.begin();
  sensors.setResolution(10);

  connectWiFi();
  initNTP();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
    setRGBColor(20, 20, 20);
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      delay(5000);
      return;
    }
    setRGBColor(0, 0, 0);
  }

  int gsrRaw = analogRead(GSR_PIN);
  float gsrVoltage = gsrRaw * (3.3 / 4095.0);

  float dhtHumidity = dht.readHumidity();
  float dhtTemperatureC = dht.readTemperature();

  sensors.requestTemperatures();
  float ds18b20TemperatureC = sensors.getTempCByIndex(0);

  Serial.print("GSR Raw: "); Serial.print(gsrRaw); Serial.print(" ("); Serial.print(gsrVoltage); Serial.println("V)");
  
  if (isnan(dhtHumidity) || isnan(dhtTemperatureC)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("DHT Humidity: "); Serial.print(dhtHumidity); Serial.print(" %");
    Serial.print("  DHT Temperature: "); Serial.print(dhtTemperatureC); Serial.println(" °C");
  }

  if (ds18b20TemperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("Failed to read from DS18B20 sensor!");
  } else {
    Serial.print("DS18B20 Temperature: "); Serial.print(ds18b20TemperatureC); Serial.println(" °C");
  }

  if (gsrRaw > GSR_THRESHOLD_ALERT) {
    setRGBColor(255, 0, 0);
    Serial.println("GSR: ALERT! (Red LED)");
  } else if (gsrRaw > GSR_THRESHOLD_MODERATE) {
    setRGBColor(255, 165, 0);
    Serial.println("GSR: Moderate (Orange LED)");
  } else {
    setRGBColor(0, 255, 0);
    Serial.println("GSR: Normal (Green LED)");
  }

  if (millis() - lastSendTime >= sendInterval) {
    bool dataReadyToSend = true;
    
    if (isnan(dhtHumidity) || isnan(dhtTemperatureC) || ds18b20TemperatureC == DEVICE_DISCONNECTED_C) {
      Serial.println("Skipping data send due to invalid sensor readings.");
      dataReadyToSend = false;
    }

    if (dataReadyToSend) {
      long currentUnixTimestamp = getUnixTimestamp();
      if (currentUnixTimestamp > 0) {
        sendDataToAWS(gsrVoltage, dhtTemperatureC, dhtHumidity, ds18b20TemperatureC, currentUnixTimestamp);
      } else {
        Serial.println("NTP time not synced yet, cannot send data. Retrying NTP sync.");
        initNTP();
      }
    }
    lastSendTime = millis();
  }

  delay(1000);
}

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 40) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi after multiple retries.");
  }
}

void initNTP() {
  Serial.println("Initializing NTP client for time synchronization...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  int attempts = 0;
  while (!time(nullptr) && attempts < 15) {
    Serial.print(".");
    delay(1000);
    attempts++;
  }
  if (time(nullptr)) {
    Serial.println("\nTime synced successfully from NTP server!");
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    Serial.printf("Current time: %s", asctime(&timeinfo));
  } else {
    Serial.println("\nFailed to obtain time from NTP server.");
  }
}

long getUnixTimestamp() {
  time_t now;
  time(&now);
  return now;
}

void setRGBColor(int red, int green, int blue) {
  red = constrain(red, 0, 255);
  green = constrain(green, 0, 255);
  blue = constrain(blue, 0, 255);

  analogWrite(LED_R_PIN, 255 - red);
  analogWrite(LED_G_PIN, 255 - green);
  analogWrite(LED_B_PIN, 255 - blue);
}

void sendDataToAWS(float gsrVoltage, float dhtTemp, float dhtHum, float dsTemp, long timestamp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send data: Wi-Fi not connected.");
    return;
  }

  HTTPClient http;
  http.begin(awsApiEndpoint);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(256);
  doc["timestamp"] = timestamp;
  doc["gsr"] = gsrVoltage;
  doc["temp_dht"] = dhtTemp;
  doc["hum_dht"] = dhtHum;
  doc["temp_ds18b20"] = dsTemp;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  Serial.print("Sending JSON: ");
  Serial.println(jsonPayload);

  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    String response = http.getString();
    Serial.println("Server Response: " + response);
  } else {
    Serial.printf("HTTP POST Error: %s (Code: %d)\n", http.errorToString(httpResponseCode).c_str(), httpResponseCode);
  }

  http.end();
}
