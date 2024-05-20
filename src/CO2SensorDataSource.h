#pragma once
#include "IDataSource.h"
#include "SparkFun_SCD4x_Arduino_Library.h"

class CO2SensorDataSource : public IDataSource
{
public:
    CO2SensorDataSource();
    bool isDataReady() override;
    std::map<std::string, std::string> getData() override;

private:
    SCD4x co2Sensor;
};