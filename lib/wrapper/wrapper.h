#pragma once
#include <Arduino.h>

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
#include "logger.h"
#include "./modules/mqttConfigurator/mqttConfig.h"

class espOS {
    private:
        
        Filemanager* _FM;
        WiFiManager* _Wifi;
        Network* _Network;
        NTPManager* _Ntp;
        OTA_Manager* _OTA;
        ServiceHandler* _serviceHandler;
        ClassModuleMaster* _Watcher;
        MQTTHandler* _mqttHandler;
        voltageDetector* _voltageDetector;
        SysLogger* _initLogger;
        ESPOS_Module_MQTTConfig* _mqttConfigurator;

        const char* cssDir = "config/os/web/";

    public:
    LED* _WorkLed;
        LED* _WifiLed;
        LED* _errorLed;
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
            _initLogger = new SysLogger("espOS");
            _mqttConfigurator = new ESPOS_Module_MQTTConfig(_mqttHandler, _FM, _Wifi);
        }   

        Filemanager* getFilemanagerObj();
        WiFiManager* getWiFimanagerObj();
        Network* getNetworkManagerObj();
        NTPManager* getNTPManagerObj();
        ServiceHandler* getServiceHandlerObj();
        MQTTHandler* getMqttHandler();
        OTA_Manager* getOtaManagerObject();
        voltageDetector* getVoltageDetectorObj();
        ESPOS_Module_MQTTConfig* getModuleMQTTConfig()
        {
            return this->_mqttConfigurator;
        }
        void addModuleToWatchdog(ClassModuleSlave* newModule);


        //control Class
        void disableLeds();
        void enableLeds();


        /*
            Website Stuff
        */
        void handleMainStateSite();
        void handleInfoSite();
        void handleLogSite();
        void handleControlSite();
        void handleWifiSite();
        void handleUpdateSite();

        //GraphicStuff
        void sendWebsiteLogo();
        void sendSpecIcons();


        //CSS Stuff
        void sendspecexpcss();
        void sendSpecCss();
        void sendownCSS();
        /*
            Special WrapperClass Stuff

        */
       void mqttOSCommands();
       void showStorage();

        void begin();
        void run();
};