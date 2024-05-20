#include "CO2SensorDataSource.h"

CO2SensorDataSource::CO2SensorDataSource()
{
    if (co2Sensor.begin() == false)
    {
        Serial.println("Sensor not detected. Please check wiring. Freezing...");
        while (1)
            ;
    }
}

bool CO2SensorDataSource::isDataReady()
{
    return co2Sensor.readMeasurement();
}

std::map<std::string, std::string> CO2SensorDataSource::getData()
{
    std::map<std::string, std::string> values;
    char buffer[8];

    const uint16_t co2ppm = co2Sensor.getCO2();
    itoa(co2ppm, buffer, 10);
    values["co2Monitor/co2Ppm"] = buffer;

    const float tempC = co2Sensor.getTemperature();
    dtostrf(tempC, 0, 1, buffer);
    values["co2Monitor/tempC"] = buffer;

    const float humidityPercent = co2Sensor.getHumidity();
    dtostrf(humidityPercent, 0, 1, buffer);
    values["co2Monitor/humidityPercent"] = buffer;

    return values;
}