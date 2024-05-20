#pragma once
#include <map>
#include <stdint.h>
#include <vector>

#include <WiFi.h>
#include <PubSubClient.h>

#include "IDataSource.h"

struct WiFiSettings
{
    const std::string ssid;
    const std::string password;
    const uint32_t timeout;
};

struct MqttSettings
{
    const std::string hostname;
    const uint16_t port;
    const std::string id;
    const std::string username;
    const std::string password;
};

typedef std::function<void(const std::string &, const std::string &)> DataListenerFunc;

class EspMqttMonitor
{
public:
    EspMqttMonitor(const WiFiSettings wifiSettings, const MqttSettings mqttSettings);
    void init();
    void loop();
    void addDataSource(const std::string &name, IDataSource *pDataSource);
    void addDataListener(const std::string &topicName, DataListenerFunc callback);

private:
    WiFiSettings _wifiSettings;
    WiFiClient _wifiClient;
    MqttSettings _mqttSettings;
    PubSubClient _mqttClient;
    std::map<std::string, IDataSource *> _dataSources;
    std::map<std::string, std::vector<DataListenerFunc>> _dataListeners;

    void _reconnectWifi();
    void _reconnectMqtt();
};