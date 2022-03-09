#pragma once
#ifndef fallbackDefines
#define fallbackDefines
/*
    Fallback config Files 
    In Case Filesystem is broken
*/

#define compileLoggingClass
#define compileLoggingClassOnlyError

//DebugLevelCompiledInFW
/*
    0 = no Logging
    1 = Trace and +
    2 = Debug and +
    3 = Info and +
    4 = Warn and +
    5 = Error and +
    6 = Only Critical
*/
#define logLevel 2

#define DEFAULTBAUDRATE 921600

//General defines
#define usedChip "esp8266"
#define deviceVersion "0.5.4"


//VERSION CONF
#define currentSFVersion "0.5.4" //Software version
#define currentFSVersion "1.0" //Filesystem version
#define currentDeviceType "thermometer" //device type -> get correct Release from server
#define currentChipType "ESP8266" //chip type


//ConfigFilePaths
#define setupConfig "config/setup.json"
#define mainConfig "config/mainConfig.json"
#define wifiConfig "config/wifiConfig.json" //contains all important settings for wifi Communication (e.g SSID, PSK, APSSID, APPSK, connectionTimeout (AP restart for new SSID regis), staEnabled, )
#define networkConfig "config/networkConfig.json" //contain all important configuration for networking (e.g hostname, manual set ipCOnfig and DNS Server)
#define LEDConfig "config/LED.json" //contains pin number of all LED's installed on this device (default names: wifiLED, wifiAPLED, errorLED, workLED, updateLED)
#define integratedDeviceConfig "config/devices" //contains pin numbers of all internal devices (sensors, button etc.) hard attached to device (soldered) (e.g - wifiButton, controlButton)
#define DNSConfig "config/DNS.json" //contains all registerd DNS information
#define OTAConfig "config/OTA.json"
#define NodeworkConnectorConfig "config/nodeworkConnector.json"
#define moduleSiteIndex "config/moduleSites.json" //contains function for sending moduleConfSite (List as JSON)
#define MQTTConfig "config/mqtt.json"

#define VERSIONFILE "config/version.json" //contains installed FS and SF version - and avail.


#define offeredServiceConfig "config/services/offered.json" //contains all device offered services -> Other hosts can call this file and register the device as service offerer
#define registeredServiceConfig "config/services/registered.json" //contains all registered services offered by other devices -> Other hosts can call this file to find asap a service in their network
#define serviceOfferConfigFile "config/services/config.json" //contains configuration for service manager(e.g serviceRegisterServer -> all services can be registrated on one host, no broadcast is needed in network)


//from class OTA Manager
#define defaultUpdateServer "http://10.0.0.1"
#define defaultUpdateServerUpdateUri "/nodework_device_controller.php/OTA"
#define defaultUpdateServerPort "80"
#define defaultUpdateSearch "false"
#define defaultAutoUpdate "false"
#define defaultOTAUseHTTPS "false"
#define defaultUpdateSoftwareVeresion "0.1"
#define defaultUpdateCheckDelay "600" //every 10min

#define defaultUpdateToken "default"
#define defaultUpdatePass "default"


#define classCheckIntervall 100


//from WiFi Class
#define defaultSSID "newNodeworkDevice"
#define defaultPSK "k2(j91J24"

#define defaultConnectTimeout 60
#define DEFAULTWIFICONNECTIONTIMEOUT 8000
#define defaultLEDInstalled true 

#define defaultAPIP 10,0,0,1
#define defaultAPGateway 10,0,0,1
#define defaultAPSubnet 255,255,255,0


//Wifi Class LED configs
#define wifi_connection_lost 400
#define wifi_connect_failed 500
#define wifi_idle 2000
#define wifi_no_ssid_avail 1000
#define wifi_disconnected 800
#define wifi_scan_completed 100

#define wifiNotConfigured 200
#define wifiNotConfiguredDeviceConnectedToAP 600


//Logging class

#define DefinedserialLoggingLevel 1 //Trace
#define DefinedserialLoggingEnabled true

#define DefinedFileLoggingLevel 5 //Error
#define DefinedFileLoggingEnabled false

#define DefinedMQTTLogingLevel 3 //Info
#define DefinedMQTTLoggingEnabled false


//Nodework Connector
#define defaultNodeworkConnectorUpdateServer "http://10.0.0.1"
#define defaultNodeworkConnectorUpdateServerRegisterPath "/nodework_device_controller.php/device/register"
#define defaultNodeworkConnectorUpdateServerUpdateUri "/nodework_device_controller.php/device/update"
#define defaultNodeworkConnectorUpdateServerPort "80"

#define defaultNodeworkConnectorUpdateToken "default"
#define defaultNodeworkConnectorUpdatePass "default"

#define defaultNodeworkConnectorUseHTTPS "false"
#define defaultNodeworkConnectorUsedChip "esp8266"
#define defaultNodeworkConnectorAlreadyRegistered "false"
#define defaultNodeworkConnectorDeviceVersion "1.0"
#define defaultNodeworkConnectorDeviceName "none"

#define defaultFailedToRegisterTimeout 600000


//LED Config
#define defaultWiFiLEDPin 12 // -> this Pin is saved in LED config
#define defaultUpdateLEDPin 13 // -> saved in LED Config
#define defaultErrorLEDPin 15 //saved in LED COnfig

//Other PIN Config
#define thermometerPinSCL 

//MQTT
#define MQTTReconnectDelay 60 //sec
#define MQTTCallbackRemoveDelay 5 //sec -> After this time a callback is removed from queue






/*
    COMPILE STUFF

*/

/*
            LOGGING
*/

/*
0 -> noLogging
- 1 -> trace
- 2 -> debug
- 3 -> info
- 4 -> warn
- 5 -> error
- 6 -> fatal

*/

#ifdef logLevel
    #if logLevel != 0
    #if logLevel <= 6
        #define createLogger(className) SysLogger* logger = new SysLogger(className);
        #define compileLoggingClassCritical
        #define logCritical(function,message) logger->logIt(function, message, 6);
    #if logLevel <= 5
        #define compileLoggingClassError
        #define logError(function,message) logger->logIt(function, message, 5);
    #if logLevel <= 4
        #define compileLoggingClassWarn
        #define logWarn(function,message) logger->logIt(function, message, 4);
    #if logLevel <= 3
        #define compileLoggingClassInfo
        #define logInfo(function,message) logger->logIt(function, message, 3);
    #if logLevel <= 2
        #define compileLoggingClassDebug
        #define logDebug(function,message) logger->logIt(function, message, 2);
    #if logLevel <= 1
        #define compileLoggingClassTrace
        #define logTrace(function,message) logger->logIt(function, message, 1);
    #endif 
    #endif 
    #endif 
    #endif 
    #endif 
    #endif
    #endif

    #ifndef logTrace
        #define logTrace(function, message)
    #endif
    #ifndef logDebug
        #define logDebug(function,message)
    #endif
    #ifndef logInfo
        #define logInfo(function,message)
    #endif
    #ifndef logWarn
        #define logWarn(function,message)
    #endif
    #ifndef logError
        #define logError(function,message)
    #endif
    #ifndef logCritical
        #define logCritical(function,message)
    #endif        
#endif
          
#endif