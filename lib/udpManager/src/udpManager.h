#ifndef J54J6_udpManager
#define j54J6_udpManager

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "logging.h"
#include "errorHandler.h"
#include "filemanager.h"
#include "wifiManager.h"



struct udpPacketResolve {
    IPAddress remoteIP;
    int remotePort = -1;
    int paketSize = 0;
    String udpContent = "NULL";

    void resetPack()
    {
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

class udpManager : public ErrorSlave {
    private:
        const char* className = "udpManager";
        bool classDisabled = false;
        bool udpListenerStarted = false;
        int listenPort;

        IPAddress broadcastIP = IPAddress(255,255,255,255); //only for first Time later this Address will dynamically changed per subnet
        WiFiUDP udpHandler;
        WiFiManager* wifiManager;
        udpPacketResolve lastContent;

    public:
        udpManager(WiFiManager* wifiManager, int port);
        ~udpManager();
        
        //get stuff
        int getListenPort();
        int getLocalOutPort();
        udpPacketResolve* getLastUDPPacketLoop();

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
        void pauseClass();
        void restartClass();
        void continueClass();
};


#endif