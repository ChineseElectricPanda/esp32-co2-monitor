#include <Arduino.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>
#include "SparkFun_SCD4x_Arduino_Library.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define WIFI_CONNECT_TIMEOUT_MS 10000

#define MQTT_SERVER "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

#define SERVO_PIN 27
#define CO2_PPM_THRESHOLD 1000
#define CO2_PPM_BUFFER 50

SCD4x co2Sensor;
Servo servo;
WiFiClient wifiClient;

PubSubClient mqttClient(wifiClient);

bool isAlertTriggered = false;

void connectWifi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t timeout = millis() + WIFI_CONNECT_TIMEOUT_MS;

  while (!WiFi.isConnected() && millis() < timeout)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.isConnected())
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println();
    Serial.print("Connecting to WiFi timed out.");
  }
}

void connectMqtt()
{
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  while (!mqttClient.connected())
  {
    if (mqttClient.connect("CO2 Monitor", MQTT_USERNAME, MQTT_PASSWORD))
    {
      Serial.println("MQTT client connected.");
    }
    else
    {
      delay(5000);
    }
  }
}

void setup()
{
  servo.attach(SERVO_PIN);
  servo.write(0);
  Serial.begin(115200);
  Wire.begin();

  //.begin will start periodic measurements for us (see the later examples for details on how to override this)
  if (co2Sensor.begin() == false)
  {
    Serial.println("Sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }

  // The SCD4x has data ready every five seconds
}

void loop()
{
  if (!WiFi.isConnected())
  {
    connectWifi();
  }

  if (!mqttClient.connected())
  {
    connectMqtt();
  }

  if (co2Sensor.readMeasurement()) // readMeasurement will return true when fresh data is available
  {
    Serial.println();

    char buffer[8];
    const uint16_t co2ppm = co2Sensor.getCO2();
    const float tempC = co2Sensor.getTemperature();
    const float humidityPercent = co2Sensor.getHumidity();

    itoa(co2ppm, buffer, 10);
    mqttClient.publish("co2Monitor/co2Ppm", buffer);

    dtostrf(tempC, 0, 1, buffer);
    mqttClient.publish("co2Monitor/tempC", buffer);

    dtostrf(humidityPercent, 0, 1, buffer);
    mqttClient.publish("co2Monitor/humidityPercent", buffer);

    Serial.print("CO2(ppm):");
    Serial.print(co2ppm);

    Serial.print(F("\tTemperature(C):"));
    Serial.print(tempC, 1);

    Serial.print(F("\tHumidity(%RH):"));
    Serial.print(humidityPercent, 1);

    Serial.println();

    if (isAlertTriggered && co2ppm < CO2_PPM_THRESHOLD - CO2_PPM_BUFFER)
    {
      servo.write(0);
      isAlertTriggered = false;
    }
    else if (!isAlertTriggered && co2ppm > CO2_PPM_THRESHOLD + CO2_PPM_BUFFER)
    {
      servo.write(180);
      isAlertTriggered = true;
    }
  }
  else
    Serial.print(F("."));

  delay(500);
}
