#include "EspMqttMonitor.h"

EspMqttMonitor::EspMqttMonitor(
    const WiFiSettings wifiSettings,
    const MqttSettings mqttSettings) : _wifiSettings(wifiSettings),
                                       _wifiClient(),
                                       _mqttSettings(mqttSettings),
                                       _mqttClient(_wifiClient)
{
}

void EspMqttMonitor::init()
{
}

void EspMqttMonitor::loop()
{
    if (!WiFi.isConnected())
    {
        _reconnectWifi();
    }

    else if (!_mqttClient.connected())
    {
        _reconnectMqtt();
    }
    else
    {
        for (const auto &[dataSourceName, dataSource] : _dataSources)
        {
            if (dataSource->isDataReady())
            {
                std::map<std::string, std::string> data = dataSource->getData();
                for (const auto &[topicName, topicValue] : data)
                {
                    _mqttClient.publish(topicName.c_str(), topicValue.c_str());
                    for (const auto listener : _dataListeners[topicName])
                    {
                        listener(topicName, topicValue);
                    }
                }
            }
        }
    }
}

void EspMqttMonitor::addDataSource(const std::string &name, IDataSource *pDataSource)
{
    _dataSources[name] = pDataSource;
}

void EspMqttMonitor::addDataListener(const std::string &topicName, DataListenerFunc callback)
{
    _dataListeners[topicName].push_back(callback);
}

void EspMqttMonitor::_reconnectWifi()
{
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(_wifiSettings.ssid.c_str());

    WiFi.begin(_wifiSettings.ssid.c_str(), _wifiSettings.password.c_str());

    const uint32_t timeout = millis() + _wifiSettings.timeout;

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

void EspMqttMonitor::_reconnectMqtt()
{
    _mqttClient.disconnect();
    _mqttClient.setServer(_mqttSettings.hostname.c_str(), _mqttSettings.port);
    while (!_mqttClient.connected())
    {
        if (_mqttClient.connect(_mqttSettings.id.c_str(), _mqttSettings.username.c_str(), _mqttSettings.password.c_str()))
        {
            Serial.println("MQTT client connected.");
        }
        else
        {
            delay(5000);
        }
    }
}