#ifndef J54J6_SIMPLEWIFIMANAGER_H
#define J54J6_SIMPLEWIFIMANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>




#include "led.h"
#include "logging.h"
#include "errorHandler.h"

#define WiFiCheckDelay 50
#define FPM_SLEEP_MAX_TIME 0xFFFFFFF


//WiFi Libary Defined
#define WiFiNoShieldErrorLedDelay 300
#define WiFiIdleStatusWiFiLedDelay 100
#define WiFiNoSSIDAvalLedDelay 1000


struct macAdress {
    uint8_t macAddr[6];  
};

//typedef std::function<void(bool)> solverfkt;

class WiFiManager : public ErrorSlave {
/* WifiConState 
 *    State: 
 *    0 = WiFi disabled
 *    1 = WiFi connected
 *    2 = WiFi station
 *    3 = WiFi no Shield
 *    4 = WiFi connection failed
 *    5 = WiFi Idle mode
 *    6 = WiFi noSSID availiable
 *    7 = man. disconnect
 *    8 = WiFi only enabled but no user connect wanted
 *    9 = WiFi AP - no user
 *   10 = WiFI AP - user connected (1 - 8 - defualt: max 4 user)
 */
    private:
        //Class parameter
        const char* className = "wifiManager";
        bool lockClass = false;
        bool overrideSettingsToPreventError = true;
        ulong lastCall; //for sim. multithreading to handle WifiCheckDelay (how often Wifi State and params will checked per Second)
        ulong callPerSecond = 0;
        int checkDelay = 50; //check class every 50ms
        //classErrorReport error; - replaced by ErrorSlave Class



        // Interface States
        bool shieldState = false;
        bool apActive = false;
        bool staActive = false;

        //Hardware Handler
        LED* wifiLed;


        //Wifi Information
        uint32_t deviceMac;


        //internal Class function
        void internalControl();
    public:
        //Constructor
        WiFiManager(); //no LED only voidLed
        WiFiManager(LED *newWifiLed);
        //WiFiManager(LED &newWifiLed, LED &newErrorLed);


        //getStuff
        bool getShieldState(); //return Wifi is Enabled or Disabled
        bool getOverrideSettingsToPreventError(); //change autoFix state (for simple syntax errors...)
        bool getLockClass(); //get State of class
        uint32_t getDeviceMac();
        int getCheckDelay();
        ulong getCallPerSecond();
        classErrorReport getCurrentErrorState();

        bool getWiFiAutoConnect();
        bool getWiFiAutoReconnect();
        const char* getWiFiHostname();
        String getWiFiHostnameAsString();

        macAdress getStationMac();
        String getStationMacAsString();
        const char* getStationMacAsChar();
        wl_status_t getWiFiState();
        String getBSSID();
        uint8_t* getBSSIDAsInt();
        int32_t getRSSI();

        //set Stuff
        void setShieldState(bool newState);
        void setOverrideSettingsToPreventError(bool newValue);
        void setWifiMode(short newMode = 1);
        void setCheckDelay(int newValue);
        void setLockClass(bool newValue);

        bool setWiFiAutoConnect(bool newValue);
        bool setWiFiAutoReconnect(bool newValue);
        bool setWiFiHostname(const char* hostname);
        bool setWiFiConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = (uint32_t)0, IPAddress dns2 = (uint32_t)0);


        //General functionalities
        void enableWiFi(WiFiMode_t mode = WIFI_AP_STA);
        void disableWiFi();
        void setWiFiMode(WiFiMode_t mode);


        //AP Stuff
        bool startWifiAP(const char *ssid, const char *passwd, int hidden = 0, int channel = 1);
        bool stopWifiAP(bool wifioff = true);
        bool configWiFiAP(IPAddress localIp, IPAddress gateway, IPAddress subnet);
        uint8_t getConnectedStations();


        //Station Stuff
        bool startWifiStation(const char* ssid, const char* passwd, WiFiMode_t mode = WIFI_STA, int32_t channel = 0, const uint8_t *bssid = (const uint8_t* )__null, bool connect = true);
        bool stopWifiStation(bool wifioff = true);

        //State functionalities
        bool wifiSTAIsConnected();
        bool wifiAPUserConnected();
        void setOpticalMessage(wl_status_t currentState);

        //Class Handler
        void run();

        //inherited ErrorSlave
        void startClass();
        void stopClass();
        void pauseClass();
        void restartClass();
        void continueClass();
};
#endif
