#pragma once
#ifndef J54J6_udpManager
#define J54J6_udpManager

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "filemanager.h"
#include "wifiManager.h"
#include "moduleState.h"


struct udpPacketResolve {
    IPAddress remoteIP;
    int remotePort = -1;
    int paketSize = 0;
    String udpContent = "NULL";

    void resetPack()
    {
        Serial.println(F("Reset called!"));
        this->paketSize = 0;
        this->remoteIP = IPAddress(0,0,0,0);
        this->udpContent = "NULL";
        this->remotePort = -1;
    }

    String returnBlank()
    {
        String output;

        for(int i = 0; i < this->paketSize; i++)
        {
            output += udpContent[i];
        }
        return output; 
    }

    void clean()
    {
        String cache;

        for(int i = 0; i < this->paketSize; i++)
        {
            cache += udpContent[i];
        }
        this->udpContent = cache;
    }
};

class WiFiUDP;
class WiFiManager;
class udpPacketResolve;
class SysLogger;
class ClassModuleSlave;

class udpManager {
    private:
        const char* className = "udpManager";
        bool classDisabled = false;
        bool udpListenerStarted = false;
        int listenPort;

        IPAddress broadcastIP = IPAddress(255,255,255,255); //only for first Time later this Address will dynamically changed per subnet
        WiFiUDP udpHandler;
        WiFiManager* wifiManager;
        udpPacketResolve lastContent;
        SysLogger logging;
        ClassModuleSlave classControl = ClassModuleSlave("udpManager", 20);

    public:
        udpManager(Filemanager* FM, WiFiManager* wifiManager, int port);
        ~udpManager();
        
        //get stuff
        int getListenPort();
        int getLocalOutPort();
        udpPacketResolve* getLastUDPPacketLoop();
        
        ClassModuleSlave* getClassModuleSlave()
        {
            return &classControl;
        }
        //set stuff - nothing 


        //control
        bool begin();
        void stop();
        void sendUdpMessage(const char* workload, IPAddress ip, int port);


        //loop
        void run();


        /*
        Inherited ErrorHandling
       */
        void startClass();
        void stopClass();
        void restartClass();
};


#endif