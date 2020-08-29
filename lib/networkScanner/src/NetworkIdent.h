#ifndef J54J6_NetworkIdent
#define J54J6_NetworkIdent

#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#include "logging.h"
#include "errorHandler.h"
#include "filemanager.h"
#include "wifiManager.h"
#include "udpManager.h"


typedef std::function<void()> handlerFunction;

struct networkIdentResolve  {
    bool result = false;
    IPAddress fromIP;
    int fromPort = -1;
    macAdress fromMAC;
};


class NetworkIdent : public ErrorSlave {
    private:
        bool classDisabled = false;

        const char* className = "NetworkIdent";
        const char* serviceListPath = "/config/networkIdent/services.json"; //saved as {serviceType, port}
        int networkIdentPort = 63547;
        
        const char* serviceConfigBlueprint[1][2] = {{"NetworkIdent", "63547"}}; //port of network scanner

        IPAddress broadcastIP = IPAddress(255,255,255,255); //only for first Time later this Address will dynamically changed per subnet
        Filemanager* FM;
        WiFiManager* wifiManager;
        udpManager udpControl = udpManager(this->wifiManager, this->networkIdentPort);

    public:
        NetworkIdent(Filemanager* FM, WiFiManager* wifiManager);

        //control
        bool beginListen();
        void stopListen();

        bool checkForService(const char* serviceName); //check if the serviceList-Json File contains a key with <<serviceName>>
        bool addService(const char* serviceName, int port); //try to add service to JsonFile
        bool delService(const char* ServiceName); //delete key from Json File - delete service


        //send stuff
        void sendServiceList(IPAddress ip, int port);
        void sendIP(IPAddress ip, int port);
        void sendHostname(IPAddress ip, int port);
        void sendMAC(IPAddress ip, int port);


        //search Stuff
        networkIdentResolve searchForService(const char* serviceName);

        //helper
        bool createConfigFile();
        
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