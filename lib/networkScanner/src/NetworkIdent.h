#ifndef J54J6_NetworkIdent
#define J54J6_NetworkIdent

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "logging.h"
#include "errorHandler.h"
#include "filemanager.h"
#include "wifiManager.h"
#include "udpManager.h"


typedef std::function<void()> handlerFunction;
class NetworkIdent : public ErrorSlave {
    private:
        bool classDisabled = false;

        const char* className = "NetworkIdent";
        const char* serviceListPath = "/config/networkIdent/services.json"; //saved as {serviceType, port}
        int networkIdentPort = 63547;
        
        const char* serviceConfigBlueprint[1][2] = {{"NetworkIdent", "63547"}}; //port of network scanner

        IPAddress broadcastIP = IPAddress(255,255,255,255); //only for first Time later this Address will dynamically changed per subnet
        Filemanager* FM;

    public:
        NetworkIdent(WiFiManager* wifiManager, Filemanager* FM, const char* deviceName);

        //set stuff
        void setDeviceName(const char* newDeviceName);
        //get stuff
        const char* getDeviceIdentName();
        int getLocalOutPort();
        udpPacketResolve* getLastUDPPacketLoop();
        
        //control
        bool begin(int port = -1);
        void stop();


        bool createConfigFile();
        bool checkForService(const char* serviceName);
        bool addService(const char* newServiceName, int usedPort);
        bool delService(const char* serviceName);

        void searchForDeviceName(const char* deviceName);
        void searchForService(const char* service);

        void sendUdpMessage(const char* workload, IPAddress ip, int port);

        void returnMyDeviceName();
        //void returnMyServices();
        void returnMyIP();

        //loop
        void loop();

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