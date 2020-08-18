#ifndef J54J6_NetworkIdent
#define J54J6_NetworkIdent
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "logging.h"
#include "errorHandler.h"
#include "filemanager.h"
#include "wifiManager.h"

struct udpPacketResolve {
    IPAddress remoteIP;
    int remotePort;
    int paketSize;
    char udpContent[512];
};

class NetworkIdent : public ErrorSlave {
    private:
        const char* className = "NetworkIdent";
        const char* deviceIdentName = "notSet";
        const char* serviceListPath = "/config/networkIdent/services.json"; //saved as {serviceType, port}
        int port;
        bool udpListenerStarted = false;

        IPAddress broadCastIP = IPAddress(255,255,255,255); //only for first Time later this Address will dynamically changed per subnet
        WiFiUDP udpHandler;
        Filemanager* FM;
        WiFiManager* wifiManager;

        udpPacketResolve lastContent;
    public:
        NetworkIdent(WiFiManager* wifiManager, Filemanager* FM, const char* deviceName, int port = 62309);

        //set stuff

        //get stuff
        const char* getDeviceIdentName();
        int getLocalOutPort();
        udpPacketResolve* getLastUDPPacketLoop();
        
        //control
        bool begin(int port = -1);
        void stop();


        bool checkForService(const char* serviceName);
        bool addService(const char* newServiceName, int usedPort);
        bool delService(const char* serviceName);
        bool serviceAvail(const char* serviceName);

        void searchForDeviceName(const char* deviceName);
        void searchForFunctionalities(const char* functionalitiy);

        void sendUdpMessage(const char* workload, IPAddress ip);

        void returnMyDeviceName();
        void returnMyFuntionalitiy();
        void returnMyIP();

        //loop
        void loop();
};

#endif