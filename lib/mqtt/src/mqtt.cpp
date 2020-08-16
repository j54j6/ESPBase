#include "mqtt.h"

//extra Stuff
bool J54J6_MQTT::configCheck()
{
    if(FM->fExist(configFile))
    {
        return true;
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "configFile already exist - SKIP!");
        #endif
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "configFile don't exist - try to create");
        #endif

        if(!FM->createFile(configFile))
        {
            #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "Can't create File", 2);
            #endif
            error.error = true;
            error.ErrorCode = 321;
            error.message = "Can't create Config File!";
            error.priority = 5;
            return false;
        }
        
        if(!FM->writeJsonFile(configFile, configFallback, 0))
        {
            #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "Can't write in File", 2);
            #endif
            error.error = true;
            error.ErrorCode = 322;
            error.message = "Can't write in File!";
            error.priority = 5;
            return false;
        }
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "Config check successfull");
            #endif
        return true;
    }
}


//Constructor

J54J6_MQTT::J54J6_MQTT(Filemanager* FM, WiFiManager* wifiManager)
{
    this->FM = FM;
    this->wifiManager = wifiManager;
}

void J54J6_MQTT::begin(const char* hostname)
{
    MQTTActive = true;
    mqttHandlerClient.begin("192.168.178.1", wifiManager->getWiFiClient());
}

void J54J6_MQTT::begin(const char* hostname, int port)
{
    MQTTActive = true;
}



void J54J6_MQTT::run()
{
    if(!mqttHandlerClient.loop())
    {
        this->error.error = true;
        this->error.ErrorCode = 365;
        this->error.message = mqttHandlerClient.
    }
    delay(10); //need to fix several stability problems look at https://github.com/256dpi/arduino-mqtt?utm_source=platformio&utm_medium=piohome#notes
}