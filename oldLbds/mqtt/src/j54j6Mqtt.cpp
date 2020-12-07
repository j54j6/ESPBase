#include "j54j6Mqtt.h"

MQTTHandler::MQTTHandler(Filemanager* FM, WiFiManager* wifiManager, ServiceHandler* serviceHandler)
{
    this->FM = FM;
    this->wifiManager = wifiManager;
    this->services = serviceHandler;
}