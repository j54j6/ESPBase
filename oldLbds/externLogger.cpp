#include "externLogger.h"

ExternLogger::ExternLogger(Filemanager* FM, WiFiManager* wifi, MQTTHandler* mqtt)
{
    this->FM = FM;
    this->wifi = wifi;
    this->mqtt = mqtt;
    this->macAddress = wifi->getDeviceMac();
    this->mqttTopic = "espOS/log/" + macAddress;
}

bool ExternLogger::logItMQTT(String message)
{
    if(mqtt->isConnected())
    {
        Serial.print("Log to: ");
        Serial.println(mqttTopic);
        return mqtt->publish(mqttTopic.c_str(), message.c_str());
    }
    else
    {
        return false;
    }
}