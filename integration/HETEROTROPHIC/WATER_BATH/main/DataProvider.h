// DataProvider.h
#ifndef DATA_PROVIDER_H
#define DATA_PROVIDER_H

#include <Arduino.h>
#include "MessageFormatter.h"
#include "config.h"
#include "SensorController.h"

class DataProvider {
public:
    virtual ~DataProvider() = default;
    virtual SensorData getLatestSensorData() = 0;
    virtual String getSystemStatus() = 0;
    virtual String getDeviceInfo() = 0;
};

class BioreactorDataProvider : public DataProvider {
public:
    SensorData getLatestSensorData() override;
    String getSystemStatus() override;
    String getDeviceInfo() override;

private:

};

#endif
