#pragma once
#ifndef dht11temp
#define dht11temp

#include <Arduino.h>
#include <DHT.h>
#include "mqttHandler.h"

class dht11Temp {
    private:
        float actualTemp = 0.0;
        float actualHumid = 0.0;
        bool useComputedTemp = true;
        bool fahrenheit = false;
        bool begin = false;
        
        DHT* dht;
        MQTTHandler* mqtt;

        ulong nextCall = 0;

        const char* deviceName = "Terasse";
        const char* mqttPath = "/data/report/temperature/terasse";

        int nextReportDelayIsMs = 900000; //15 minutes
    public:
        dht11Temp(MQTTHandler* mqtt, int dhtPin, bool isFahrenheit = false);
        float getTemp();
        float getHumidity();
        void setUseComputedTemp(bool newVal);
        String getTempAndHumidAsJSON();
        void setMQTT(MQTTHandler* mqtt);

        void mqttCyclicReport();
        void run();
};
#endif //dht11temp