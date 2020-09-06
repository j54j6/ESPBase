#ifndef J54J6_SERVICEHANDLER_H
#define J54J6_SERVICEHANDLER_H
#include <Arduino.h>


#include "errorHandler.h"
#include "logging.h"
#include "NetworkIdent.h"
#include "wifiManager.h"
#include "filemanager.h"


/*
    externalServiceRegistration File Blueprint:
    Filename-Syntax: <<serviceAddressListFile>>+serviceName+.json
    =>/config/serviceHandler/registered/mqtt.json

    JSON-Format:
    {
        "ip" : "<<Address>>",
        "mac" : "<<mac>>",
        "port" : <<port>>,

    }
*/

struct networkSearchCacheValueHolder {
    const char* serviceName = "n.S";
    int searchType = 0; // 0 = nothing, 1 = ip, 2 = port, 3 = mac, 4 - autoAdd
    long id = 0;
    ulong createdAt = 0;
    int deleteAfter = 20000; //10 seconds default delay
    bool isFallback = false;
    
    void reset() {
        searchType = 0;
        serviceName = "n.S";
        id = 0;
        createdAt = 0;
        isFallback = false;

        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog("Struct - networkSearchCacheValueHolder", "reset", "reset ServiceRequestCacheStruct", 0);
        #endif
    }

    void loop()
    {
        if(strcmp(serviceName, "n.S") == 0)
        {
            return;
        }
        if(millis() >= (createdAt + deleteAfter) && strcmp(this->serviceName, "n.S") != 0)
        {
            reset();
        }
    }

};

class ExternServiceHandler : public ErrorSlave {
    private:
        const char* className = "ExternServiceHandler";
        bool classDisabled = false;
        const char* serviceAddressListFile = "/config/serviceHandler/registered/";

        //you can handle up to 10 requests at the same "time" - bec. of only one core there is more like you can wait for up to 10 answers of other devices at the same time ^^ (later implemented (in V1.5+))
        networkSearchCacheValueHolder lastRequest;
        //int counter = 0;

        //handler
        Filemanager* FM;
        WiFiManager* wifiManager;
        NetworkIdent* networkIdent;


        //ControlFlow Vars

        /*
            autoAddService
        */
        bool autoAddRunning = false; //is AutoAdd already running
        
        ulong autoAddLastCall;
        int checkDelay = 3000;
        short timeoutAfter = 30000; //time after request will be deleted - (autoAdd returns 0) - lastRequest will cleared



    public:
        ExternServiceHandler(Filemanager* FM, WiFiManager* wifiManager, NetworkIdent* networkIdent);

        int getServiceDestPort(const char* serviceName); //look in configFile for preDefined Service Port if nothing found autoAdd is used
        IPAddress getServiceIP(const char* serviceName); //look in configFile for preDefined Service Address if nothing found autoAdd is used
        String getServiceMACAddress(const char* serviceName); //look in configFile for preDefined Service MAC Address if nothing foud autoAdd is used
        short autoAddService(const char* serviceName = "n.S"); //search and add service to config if found in network - return 0 if failed, returns 1 if successfull, returns 2 while working return 3 when main and fallback is defined - "Cant add new one"
        bool manAddService(const char* serviceName = "n.S", int port = -1, IPAddress ip = IPAddress(0,0,0,0), bool fallback = false, bool checkConnect = true, const char* MAC = "n.S"); //manually write Service into File e.g for webInterface 
        bool delService(const char* serviceName); //delete an external Service manually or if implemented e.g after 3 times of non reach

        //helper functions
        String getFilename(const char* servicename, bool fallback = false);


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