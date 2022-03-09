#pragma once
#ifndef J54J6_NETWORK_H
#define J54J6_NETWORK_H

#define PJON_INCLUDE_ETCP

#include <Arduino.h>
#include <avr/pgmspace.h>
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
#include "../webSrc/speccss.h"
#include "moduleState.h"

extern "C" {
  #include "user_interface.h"
}
typedef std::function<void()> webService;



class Network {
    private:
        /*
            General Class stuff
        */
        const char* configFile  = "config/network.json"; //class Intern ConfigFile
        const char* setupFile = "setup/setup.json"; //first init File wih AP credentials
        const char* className = "Network"; //only for Debugging
        const char* hostName = "newDevice"; //Network WiFi Hostname
        const char* registerHostname = "registration";
        const char* backupAPSSID = "newSensor";
        const char* backupAPPSK = "dzujkhgffzojh";

        ulong lastCall = 0;
        const int checkDelay = 50;

        IPAddress apIP = IPAddress(192,168,178,1);
        const byte DNS_PORT = 53;

        /*
            external Class Handler
        */
        LED* wifiLed; //pointer to wifiLed - without parmam there is a "void-LED" passed
        WiFiManager* wifiManager; //external wifiManager
        Filemanager* _FM;
        DNSServer dnsServer;
        ESP8266WebServer webserver;
        SysLogger logging = SysLogger("Network");
        ClassModuleSlave classControl = ClassModuleSlave("Network", checkDelay);

        bool workMode = false;
        bool classDisabled = false;
        bool webserverActive = false;


        /*
            Internal used Functions
        */
        bool createConfig();
        bool initSetup();
        bool saveCredentials(const String* ssid, const String* psk, const char* File);

        bool hostnameAlreadySet = false;
    protected:

        void sendWiFiSite();
        void sendspecexpcss();
        void sendownCSS();
        void sendSpecCss();
        void sendSpecIcons();

        void checkAndTestCredits();
        String getHTMLFormattedWiFiNetworksForSetupHandler();
        int getRSSIasQuality(int RSSI);

        void startWorking();
    public:
        Network(Filemanager* FM, WiFiManager* wifiManager);
        Network(Filemanager* FM, WiFiManager* wifiManager, LED* wifiLed);

        /*
            Modules
        */
        bool startDNSServer(uint16 port, String domainName, IPAddress resolveIP);

        /*
            Webserver functionalities
        */
        void startWebserver(int port);
        void stopWebserver();
        void addService(const char *url, webService function);
        void addNotFoundService(webService function);
        ESP8266WebServer* getWebserver();

        bool updateHostname(const char* newHostname);

        bool getDeviceIsConfigured();
        void begin();
        void run();

        ClassModuleSlave* getClassModuleSlave()
        {
            return &classControl;
        }
};

#endif