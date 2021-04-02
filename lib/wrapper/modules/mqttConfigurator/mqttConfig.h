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
        "group" : "xxxxx", //alternative to target - all devices with same group will use new config //not implemented yet
        "password" : "xxxxx", //to identifiy - only for p2p (but in general this password is useless^^ it is more like "not every program can send any request and destroy device - it is better to disable confMode when finished")
        "filename" : "", //for "set" operations you can specify a filename to change Configurationvar
        "setting" : "settingName",
        "key"   : "JSONCFG Key" //used for "changeConfig" Command
        "value" : "val"
    }

    MQTT response

    {
        "type"  : "response",
        "source" : "deviceMac",
        "sorceGroup" : "group", //not implemented yet
        "requestedCommand" : "command",
        "state" : "success" //or "failed",
        "payload" : "..."
    }

    All Commands (setting) will converted to lower case!!!

    Implemented Commands:
    - disableMQTTConfig
    - get <<filename>> AS setting: readFile, value:filename
    
    - set
        - changeConfig AS setting: changeConfig, filename: <<filename>>, value: JSONValue (only possible with JSON Configs! - use inbuilt JSON Filemanager to create JSON Config)
*/

class ESPOS_Module_MQTTConfig {
    private:
        MQTTHandler* _MQTT;
        Filemanager* _FM;
        WiFiManager* _WiFi;
        SysLogger logging = SysLogger("ESPOS_Module_MQTTConfig");
        ClassModuleSlave classControl = ClassModuleSlave("ESPOS_MQTTConfigModule", 10);

        const char* configFile = "config/os/modules/mqttConfig.json";
        const char* settingsMQTTTopic;
        bool mqttConfigEnabled = false;
        String passwd = "";
        StaticJsonDocument<300> lastCb;

    protected:
        /*
            state:
                1 = success
                0 = failed
        */
        String formatResponse(String command, short state, String payload);
        bool getInformation(StaticJsonDocument<200> lastCb);
        //bool setInformation(StaticJsonDocument<200> lastCb);

        /*
            reason
                0 => unknown command
                1 => not permitted
                2 => File not found
        */
        //void sendError(short reason = 0);

        void MQTTConfReadFile(lastMqttCallback* lastCb, String filename);
    public:
        ESPOS_Module_MQTTConfig(MQTTHandler* mqtt, Filemanager* FM, WiFiManager* wifi);
        bool begin(); // load Config
        bool checkFiles(); //check For Files
        bool setPassword(const char* newPassword);
        bool setChannel(const char* newChannel);
        bool setConfMode(bool confModeViaMQTTEnabled = false);
        void runGetCommands(lastMqttCallback* lastCb);
        void runSetCommands(lastMqttCallback* lastCb);
        void checkForCommands(lastMqttCallback* lastCallback);
        void run(bool startListen = false);

        ClassModuleSlave* getClassModuleSlave()
        {
            return &classControl;
        }

};