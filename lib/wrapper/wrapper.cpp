#include "wrapper.h"

Filemanager* espOS::getFilemanagerObj()
{
    return this->_FM;
}

WiFiManager* espOS::getWiFimanagerObj()
{
    return this->_Wifi;
}

Network* espOS::getNetworkManagerObj()
{
    return this->_Network;
}

NTPManager* espOS::getNTPManagerObj()
{
    return this->_Ntp;
}

MQTTHandler* espOS::getMqttHandler()
{
    return this->_mqttHandler;
}

OTA_Manager* espOS::getOtaManagerObject()
{
    return this->_OTA;
}

void espOS::addModuleToWatchdog(ClassModuleSlave* newModule)
{
    this->_Watcher->addModuleSlave(newModule);
}

void espOS::begin()
{
    Serial.begin(921600);
    _FM->mount();
    _serviceHandler->beginListen();
    _Network->begin();
    _mqttHandler->init();
    _Watcher->addModuleSlave(_Wifi->getClassModuleSlave());
    _Watcher->addModuleSlave(_Network->getClassModuleSlave());
    _Watcher->addModuleSlave(_mqttHandler->getClassModuleSlave());
    _Watcher->addModuleSlave(_serviceHandler->getClassModuleSlave());
}

void espOS::run() {
    this->_WifiLed->run();
    this->_WorkLed->run();
    this->_errorLed->run();
    this->_Network->run();
    this->_Ntp->run();
    this->_OTA->run();
    this->_Wifi->run();
    this->_serviceHandler->loop();
    this->_Watcher->run();
    this->_mqttHandler->run();
}
