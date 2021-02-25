#ifndef EXTERNLOGGER_H
#define EXTERNLOGGER_H
#include <Arduino.h>
#include "mqttHandler.h"
#include "filemanager.h"
#include "wifiManager.h"

class ExternLogger {
    private:
        Filemanager* FM;
        WiFiManager* wifi;
        MQTTHandler* mqtt;

        String mqttTopic;
        String macAddress;

    public:
        ExternLogger(Filemanager* FM, WiFiManager* wifi, MQTTHandler* mqtt);

        bool logItMQTT(String message);
        //bool logItMQTT(String function, String message, String logLevel);
};

#endif //EXTERNLOGGER_H