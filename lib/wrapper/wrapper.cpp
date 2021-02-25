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

voltageDetector* espOS::getVoltageDetectorObj()
{
    return this->_voltageDetector;
}

void espOS::addModuleToWatchdog(ClassModuleSlave* newModule)
{
    this->_Watcher->addModuleSlave(newModule);
}

void espOS::disableLeds()
{
    _WorkLed->disable();
}

void espOS::begin()
{
    Serial.begin(921600);
    _FM->mount();
    _initLogger->setMqttClient(_mqttHandler->getPubSubClient());
    _serviceHandler->beginListen();
    _Network->begin();
    _mqttHandler->init();
    _mqttConfigurator->begin();
    _Watcher->addModuleSlave(_Wifi->getClassModuleSlave());
    _Watcher->addModuleSlave(_Network->getClassModuleSlave());
    _Watcher->addModuleSlave(_mqttHandler->getClassModuleSlave());
    _Watcher->addModuleSlave(_serviceHandler->getClassModuleSlave());
    _Watcher->addModuleSlave(_mqttConfigurator->getClassModuleSlave());
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
    this->_voltageDetector->run();
    this->_dht11->run();
    this->_mqttConfigurator->run();
}
