#pragma once
#include <Arduino.h>

#include "logger.h"
#include "button.h"
#include "filemanager.h"
#include "led.h"
#include "moduleState.h"
#include "mqttHandler.h"
#include "network.h"
#include "ntpManager.h"
#include "ota.h"
#include "serviceHandler.h"
#include "udpManager.h"
#include "wifiManager.h"
#include "voltageDetect.h"
//#include "externLogger.h"

class WiFiManager;
class Network;
class Filemanager;
class ClassModuleSlave;
class udpManager;
class SysLogger;
class espOS {
    private:
        LED* _WorkLed;
        LED* _WifiLed;
        LED* _errorLed;
        Filemanager* _FM;
        WiFiManager* _Wifi;
        Network* _Network;
        NTPManager* _Ntp;
        OTA_Manager* _OTA;
        ServiceHandler* _serviceHandler;
        ClassModuleMaster* _Watcher;
        MQTTHandler* _mqttHandler;
        voltageDetector* _voltageDetector;
        //ExternLogger* _externLogger;

    public:
        espOS(int workLedPin = -1, int errorLedPin = -1, int wifiLedPin = -1)
        {
            _WorkLed = new LED(workLedPin); //D2
            _WifiLed = new LED(wifiLedPin); //D1
            _errorLed = new LED(errorLedPin); //D7
            _FM = new Filemanager(true);
            _Wifi = new WiFiManager(_WifiLed, _FM);
            _Network = new Network(_FM, _Wifi);
            _Ntp = new NTPManager(_FM, _Wifi);           
            _serviceHandler = new ServiceHandler(_FM, _Wifi, 30);
            _Watcher = new ClassModuleMaster(_errorLed, _WorkLed);
            _mqttHandler =  new MQTTHandler(_FM, _Wifi, _serviceHandler);
            _OTA = new OTA_Manager(_FM, _Network, _Ntp, _Wifi, _WorkLed);
            _voltageDetector = new voltageDetector(A0);
            //_externLogger = new ExternLogger(_FM, _Wifi, _mqttHandler);
        }

        Filemanager* getFilemanagerObj();
        WiFiManager* getWiFimanagerObj();
        Network* getNetworkManagerObj();
        NTPManager* getNTPManagerObj();
        ServiceHandler* getServiceHandlerObj();
        MQTTHandler* getMqttHandler();
        OTA_Manager* getOtaManagerObject();
        voltageDetector* getVoltageDetectorObj();
        void addModuleToWatchdog(ClassModuleSlave* newModule);


        //control Class
        void disableLeds();

        void begin();
        void run();
};