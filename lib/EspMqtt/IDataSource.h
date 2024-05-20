#pragma once
#include <stdint.h>
#include <map>

class IDataSource
{
public:
    virtual bool isDataReady() = 0;
    virtual std::map<std::string, std::string> getData() = 0;
};