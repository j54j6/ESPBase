#include "externLogger.h"

ExternLogger::ExternLogger(Filemanager* FM, WiFiManager* wifi, MQTTHandler* mqtt)
{
    this->FM = FM;
    this->wifi = wifi;
    this->mqtt = mqtt;
}

bool ExternLogger::logItMQTT(String message)
{
    if(mqtt->isConnected())
    {
        return mqtt->publish(mqttTopic.c_str(), message.c_str());
    }
    else
    {
        return false;
    }
}