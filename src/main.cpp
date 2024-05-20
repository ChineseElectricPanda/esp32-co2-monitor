#include <Arduino.h>
#include <ESP32Servo.h>
#include "EspMqttMonitor.h"

#include "CO2SensorDataSource.h"
#include "Secrets.h"

#define SERVO_PIN 27
#define CO2_PPM_THRESHOLD 1000
#define CO2_PPM_BUFFER 50

Servo servo;

bool isAlertTriggered = false;

EspMqttMonitor espMqttMonitor({.ssid = WIFI_SSID,
                               .password = WIFI_PASSWORD,
                               .timeout = WIFI_CONNECT_TIMEOUT_MS},
                              {.hostname = MQTT_SERVER,
                               .port = MQTT_PORT,
                               .id = "CO2 Monitor",
                               .username = MQTT_USERNAME,
                               .password = MQTT_PASSWORD});

void LoggingDataListener(const std::string &topic, const std::string &value)
{
  Serial.print(topic.c_str());
  Serial.print(value.c_str());
  Serial.println();
}

void ServoTriggerListener(const std::string &topic, const std::string &value)
{
  const uint16_t co2ppm = atoi(value.c_str());
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

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  servo.attach(SERVO_PIN);
  servo.write(0);

  espMqttMonitor.init();

  espMqttMonitor.addDataSource("CO2 Monitor", new CO2SensorDataSource());
  espMqttMonitor.addDataListener("co2Monitor/co2Ppm", LoggingDataListener);
  espMqttMonitor.addDataListener("co2Monitor/tempC", LoggingDataListener);
  espMqttMonitor.addDataListener("co2Monitor/humidityPercent", LoggingDataListener);
  espMqttMonitor.addDataListener("co2Monitor/co2Ppm", ServoTriggerListener);
}

void loop()
{
  espMqttMonitor.loop();
}
