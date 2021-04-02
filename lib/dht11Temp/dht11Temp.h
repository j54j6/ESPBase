#pragma once
#ifndef dht11temp
#define dht11temp

#include <Arduino.h>
#include <DHT.h>
#include "mqttHandler.h"
#include "filemanager.h"
#include "logger.h"

class dht11Temp {
    private:
        float actualTemp = 0.0;
        float actualHumid = 0.0;
        bool useComputedTemp = true;
        bool fahrenheit = false;
        bool classBegin = false;
        int tempCheckIntervall = 60; //seconds
        
        DHT* dht;
        MQTTHandler* mqtt;
        Filemanager* _FM;
        SysLogger logging = SysLogger("dht11TempMod");


        ulong nextCall = 0;
        const char* configFile = "config/dht11Mod.json";
        String deviceName = "";
        String mqttPath = "espOS/usedata/";
        String controlTopic = "espOS/control/mod/";

        int nextReportDelayIsMs = 900000; //15 minutes
    public:
        dht11Temp(Filemanager* _FM, MQTTHandler* mqtt, int dhtPin, bool isFahrenheit = false);
        bool createConfigFile();
        bool begin();
        float getTemp();
        float getHumidity();
        void setUseComputedTemp(bool newVal);
        String getTempAndHumidAsJSON();
        void setMQTT(MQTTHandler* mqtt);

        void mqttCyclicReport();
        bool mqttGetemperaturePerRequest(bool cyclicRequest = false);
        void run(bool forceRead = false);
};
#endif //dht11temp