#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

#include "mqttHandler.h"
#include "filemanager.h"
#include "wifiManager.h"
#include "logger.h"
#include "moduleState.h"


/*
    ConfigFile:
    
    {
        "confMode" : "false",
        "password" : "",
        "mqttChannel" : "control/config/default"
    }

    MQTT Message request (JSON)

    {
        "type" : "get" //get - get information, set - set information, response - from device
        "target" : "xx:xx:xx:xx:xx", //mac address of target device - if empty all will reached
        "group" : "xxxxx", //alternative to target - all devices with same group will use new config
        "password" : "xxxxx", //to identifiy - only for p2p (but in general this password is useless^^)
        "settings" : "[ ["setting1" : "val1"] , ["setting2": "val2"] ]"
    }

    MQTT response

    {

    }
*/

class ESPOS_Module_MQTTConfig {
    private:
        MQTTHandler* _MQTT;
        Filemanager* _FM;
        WiFiManager* _WiFi;
        SysLogger logging = SysLogger("ESPOS_Module_MQTTConfig");
        ClassModuleSlave classControl = ClassModuleSlave("ESPOS_MQTTConfigModule", 10);

        const char* configFile = "config/os/modules/mqttConfig.json";
        String basemqttConfigChannel = "control/config/";
        bool mqttConfigEnabled = false;
        String passwd = "";

    protected:
        bool getInformation(StaticJsonDocument<200> lastCb);
        //bool setInformation(StaticJsonDocument<200> lastCb);

        /*
            reason
                0 => unknown command
                1 => not permitted
        */
        //void sendError(short reason = 0);

    public:
        ESPOS_Module_MQTTConfig(MQTTHandler* mqtt, Filemanager* FM, WiFiManager* wifi);
        bool begin(); // load Config
        bool checkFiles(); //check For Files
        bool setPassword(const char* newPassword);
        bool setChannel(const char* newChannel);
        bool setConfModeEnabled(bool confModeViaMQTTEnabled = false);
        void run();

        ClassModuleSlave* getClassModuleSlave()
        {
            return &classControl;
        }

};