#pragma once
#ifndef J54J6_NETWORK_H
#define J54J6_NETWORK_H

#define PJON_INCLUDE_ETCP

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <PubSubClient.h>
//#include <PJON.h>
#include "wifiManager.h"
#include "filemanager.h"
#include "../webSrc/setupPage.h"
#include "moduleState.h"

extern "C" {
  #include "user_interface.h"
}
typedef std::function<void()> webService;
class Network
{
    /*
        Notes:
            Fkt.Nr. Placeholder:
                - 2 - (-100) - Service and Setup functionalities - only for internal Class use !DONT USE THESE NUMBERS! - in bad cases you destroy your config!
                -1 = reserved - nothing to do ->instant return
                0 = reserved
                1 - 49 = public class functionalities (e.g begin - startWifi - etc.)
                50 - 99 = Reserved to increase space defiend above for later purposes - can used by user but will removed when space is needed (please use space defined below)
                100 - 200 = internal protected class functionalities (e.g lock(time))
                1000 - 1500 = User space - for extra functionalities (use addFunctionalities(int number, void function())) - planned feature

    */
    private:
        /*
            General Class stuff
        */
        const char* configFile = "/config/network.json"; //class Intern ConfigFile
        const char* setupFile = "/setup/setup.json"; //first init File wih AP credentials
        const char* className = "Network"; //only for Debugging
        const char* hostName = ""; //Network WiFi Hostname
        const char* registerHostname = "registration";
        const char* backupAPSSID = "NodeworkNewDevice";
        const char* backupAPPSK = "NodeworkDevice1234";
        
        ulong lastCall = 0; //only for class
        int checkDelay = 20; //Check class (call run()) every 20ms
        ulong callPerSecond = 0; //performance indicator - normally 1000/checkDelay

        /*
            Fallback files - in Case there are no preflashed credentials and configFiles on ESP ROM
            usually this Files(values) don't used
        */

        const char* setupFileFallback[3][2] = {
            {"ssid", "NodeWork-FallbackDevice"},
            {"psk", "3j2!k4ji"},
            {"hostType", "hidden"}
        };

        const char* configFileFallback[12][2] = {
            {"ssid", ""},
            {"psk", ""},
            {"wiFiConfigured", "false"},
            {"state", "notConfigured"},
            {"wifiMode", "client"},
            {"wifiState", "true"},
            {"wifiAutoTime", "3600"},
            {"wifiStationDefSSID", "test"},
            {"wifiStationDefPSK", "test"},
            {"hostType", "hidden"},
            {"channel", ""},
            {"bssid", ""}
        };

        /*
            external Class Handler
        */
        LED* wifiLed; //pointer to wifiLed - without parmam there is a "void-LED" passed
        WiFiManager* wifiManager; //external wifiManager
        Filemanager* FM;
        DNSServer dnsServer;
        ESP8266WebServer webserver;
        PubSubClient mqttHandler;
        SysLogger logging;
        ClassModuleSlave classControl = ClassModuleSlave("Network", 50);
        

        /*
            Device State variables
        */
        bool deviceConfigured = false;


        /*
            Network specific Variables
        */
        const byte DNSPort = 53;
        IPAddress apIpAddress = IPAddress(172, 20, 0, 1); //only useable for setup
        IPAddress apIpGateway = IPAddress(172, 20, 0, 1);
        IPAddress apNetMsk = IPAddress(255,255,255,0); //only used for setupMode - preDefined apIP
        IPAddress dnsIP = IPAddress(172, 20, 0, 1);
        

        /*
            intern Class variables 
        */
        bool classDisabled = false; //in Case of error class can be stopped to prevent Damage or unexcepted behaviour
        bool locked = false;
        ulong unlockAt = 0;
        short runFunction = -1;
        bool webServerActive = false;
        

        /*
            private Function
        */
        bool createConfig(); //if no ConfigFile is preflashed create default
        bool createSetupFile(); //if no setupFile is pre flashed create default
        void startWorking(); //if network configured - start working Mode (default function - connect to wifi/set AP - start services - etc.)
        void startSetupMode(); //if network is not configured (normally at first start - start Setup)
        
        bool startDnsServer();
        bool startMDnsServer(const char* newHostname = "undefinedDevice");
        
        void serverHandleSetup();
        void serverHandleCaptiveNotFound();

    protected:
        void autoResetLock(); // called by run()
        void lock(ulong time); //alternative to delay();
        void internalBegin();
        void updateHostname(const char* newHostname = "");
        //bool internalDnsStart(); - replaced by startDNSServer

        
       

    public:
        Network(Filemanager* FM, WiFiManager* wifiManager);
        Network(Filemanager* FM, WiFiManager* wifiManager, LED* wifiLed);
        
        //Class Control
        void begin(); //check for Files and device State (is already Configured or first Start mode) - fktNr. 1
        void startSetup(); // fktNr. 2 -> start function -2 (startSetupMode)
        void startWifiAP(); // fktNr. 3
        void mdnsStart(); // start fktNr. 4
        void dnsStart(); // start fktNr. 5



        //get Stuff
        bool getClassDisabled();
        bool getDeviceIsConfigured() { return deviceConfigured;};
        ClassModuleSlave* getClassModuleSlave()
        {
            return &classControl;
        }

        //set Stuff
        void setClassDisabled(bool newVal);
        bool saveCredentials(const String* ssid, const String* psk, const char* File);
        
    
        /*
            Webserver functionalities
        */
        bool startWebserver(int port);
        bool stopWebserver();
        void addService(const char *url, webService function);
        void addNotFoundService(webService function);
        ESP8266WebServer* getWebserver();
        void enableCaptive();
        void disableCaptive();
        void addCaptive();

        /*
            Specific Stuff
        */

                /*
                    Setuphandler functions
                */
        String getHTMLFormattedWiFiNetworksForSetupHandler();
        int getRSSIasQuality(int RSSI);
        void checkAndTestCredits();

        /*
            RUN
        */
        void run(); //Handler function need to be called in arduino>loop()

       /*
        Inherited ErrorHandling
       */
        void startClass();
        void stopClass();
        void pauseClass();
        void restartClass();
        void continueClass();
};
#endif