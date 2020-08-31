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

struct networkAnswerResolve {
    bool changed = false;
    bool request = false; //if true this is an request, false its an Answer
    String serviceName = "n.S";
    String mac = "n.S";
    IPAddress ip = IPAddress(0,0,0,0);
    int servicePort = -1;

    void resetPack()
    {
        this->changed = false;
        this->request = false;
        this->serviceName = "n.S";
        this->mac = "n.S";
        this->ip = IPAddress(0,0,0,0);
        this->servicePort = -1;
    }
};


/*
    JsonBlueprint:
        {
            "Type: " : "Request",
            "serviceName" : "<<serviceName>>",
            "MAC" : "<<MAC-Address>>",
            "IP" : "<<ipAddress>> ", //ip of target to prevent delivering to wrong device if broadcast way used - else empty (normal case)
            "servicePort" : "<<Port>>"
        }
*/  

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
        //Constructor
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
        void searchForService(const char* serviceName, IPAddress ip = IPAddress(255,255,255,255), int port = 63547);

        //helper
        bool createConfigFile();

        /*
            Preformat and returns a formatted message to send and use with NetworkIdent

            if request = true
                you will send an request, you need to add an serviceName if not there will be "notSet" as Service
            if request = false
                yoou will send an answerMessage for a request - in this case you don't need a serviceName, it will be dropped
        */
        String formatMessage(bool request = false, const char* serviceName = "n.S", const char* MAC = "n.S", const char* ip = "n.S", int port = -1);
        networkAnswerResolve getReceivedParameters(DynamicJsonDocument* lastLoopDoc);
        
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