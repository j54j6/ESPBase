#ifndef JJMQTT
#define JJMQTT

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "wifiManager.h"
#include "filemanager.h"
#include "serviceHandler.h"

#include "errorHandler.h"
#include "logging.h"

/*
    MQTT Config Blueprint

    {
        "id" : "",
        "user" : "",
        "pass" : "",
        "willTopic" : "",
        "willQos" : "",
        "willRetain" : "",
        "willMessage" : "",
        "cleanSession" : ""
    }

*/


class testing {
    private:
        Filemanager* FM;
        WiFiManager* wifiManager;
        ServiceHandler* services;

    
    protected:
    

    public:
        testing(Filemanager* FM, WiFiManager* wifiManager, ServiceHandler* serviceHandler);

};

#endif