#ifndef J54J6_SERVICEHANDLER_H
#define J54J6_SERVICEHANDLER_H

//include Stuff
//public Libs
#include <Arduino.h> //Basic Lib
#include <ESP8266WiFi.h> //WifiControl
#include <WiFiUdp.h> //base for UDP Manager
#include <ArduinoJson.h> //useage of JSON 

//own Libs
#include "errorHandler.h" //errorHandling to Control the class and report to other classes
#include "filemanager.h" //Filemanager based on LittleFS with Config Addins 
#include "logging.h" //Serial and File Logging
#include "wifiManager.h" //control of Wifi Interface
#include "udpManager.h" //udp Manager to control and receive UDP connection/packets

/*
    networkSearchCacheValueHolder
    This struct is only used to cache all values of Network Search - funcion "autoAdd"

    only for internal useage
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




/*
    ServiceHandler Class

    Basicly an easy way to found other devices with this protocol in Network (mainly for ESP8266 and 32) - 

    This Libary offers the opportunity to discover the Network - you need to define the offerd Services on each device
    All services a device not offers can be automatically (if implemented by the hookup class) searched, saved and used - 

    This Class is as simple as possible but you will see it in the following...

    rules for create a new <<serviceName>>
        - CamelCaseWriting
        - no Space
        - the Name is not "NULL" -> this will be dropped and the service can't be found or saved
        - only ASCII letter/numbers[a-z, A-z][0-9] - tested with only letter and numbers
        - serviceName max. 20 char long - longer works too but it may cause in Problems using LittleFS
        - that's all^^ - you see very easy

    Situation:
        incoming JSON with "request" for <<serviceName>> -> another device want to find a device offering the Service <<servicename>> e.g "webserver" or "mqttserver"
            This Class do:
                -> check the syntax of the JSON (include all important keys and values)
                -> check if this Service is a self-offered Service (this device is host)
                -> check if the Service is saved as an extern Service (because of earlier useage of this service)

                If:
                    Service is local hosted and can be used by extern
                        -> send all Data needed to the requesting device
                else:
                    Service is not local hosted
                        If:
                            service is saved as an external Service
                                -> send the saved data to the other device
                -
                if the Service <<ServiceName>> is not saved as local Offered or external Service return nothing and end

        incoming JSON with "answer" for <<serviceName>> - after sending own request for <<serviceName>>
            This Class do:
                -> check the syntax of the Json (include all important keys and values)
                -> check if this Serivice is already saved as external Serivice (internal Services are "hardcoded" and normally doesnt changed (but it is possible))
                
                If:
                    Service is already saved as external Service (2 Files per Service are possible - main Cfg and Fallback - if Cfg can't be reached )
                        -> end function and drop packet
                else:
                    Service is not saved:
                        -> Try to add the Service and create new File
        
*/


/*
    ####################################
        externalServiceJson Scheme
    ####################################
        INFO: external Services devided into single Files (max. 2 per Service (mainCFG and Fallback))
    JSON-Format:
    {
        "ip" : "<<Address>>",
        "mac" : "<<mac>>",
        "port" : <<port>>,
    }

*/

/*
    ####################################
      internalOfferedServiceJson Scheme
    ####################################
        INFO: all offered Services are registered in one Single File - appended every time a new Service will be added
                the Filename is static defined
        
    JSON-Format:
    {
        "serviceName" : "port",
        "serviceName2" : "port2", 
        [...]
    }

*/    

/*
    ##############################################
        Incoming/Outgoing JSON Messages Scheme
    ##############################################
    JSON Format:
    {
        "type: " : "Request",
        "serviceName" : "<<serviceName>>",
        "MAC" : "<<MAC-Address>>",
        "IP" : "<<ipAddress>> ", //ip of target to prevent delivering to wrong device if broadcast way used - else empty (normal case)
        "servicePort" : "<<Port>>"
        "id" : "<<id>>" //id is randomGenerated by millis()+RandomNumber - used to handle multiple servicerequests if needed (later implemented)
    }
*/

class ServiceHandler : public ErrorSlave
{
    private:
        //internal Class Stuff
        const char* className = "serviceHandler"; //for Logging (class Logger.h)
        bool classDisabled = false;


        //config of Class
        int networkIdentPort = 63547;
        int timeoutAfterAutoAddWillEnd = 10000;
        const char* serviceConfigBlueprint[1][2] = {{"NetworkIdent", "63547"}}; //port of network scanner - basic Configuration


        /*  
            autoAddRUnning
                used for audoAdd function to init autoAdd or use the inherited loopFunction to resolve and use received answers
        */
        bool autoAddRunning = false;
        
        //File paths
        const char* offeredServicesPath = "/config/networkIdent/services.json"; //saved in one File as JSON -  {serviceName : port , serviceName : port , ...}
        const char* externalServicesPath = "/config/serviceHandler/registered/"; //saved in multipleFiles - every Service has its own File with the name <<serviceName>>.json or <<serviceName>>-fallback.json

        //caching for internalClass Stuff
        long lastId = 0;

        //saves the last fetched Data from UDP Manager
        StaticJsonDocument<425> udpLastReceivedDataDocument;

        //save all Values for autoAdd loop
        networkSearchCacheValueHolder lastAutoAddRequest;

        //internal Handler
        Filemanager* FM;
        WiFiManager* wifiManager;
        udpManager udpControl = udpManager(this->wifiManager, this->networkIdentPort);

    protected:
        //internal helper functions

        /*
            Preformat and returns a formatted message to send and use with NetworkIdent

            if request = true
                you will send an request, you need to add an serviceName if not there will be "notSet" as Service
            if request = false
                yoou will send an answerMessage for a request - in this case you don't need a serviceName, it will be dropped

            if generateId = true
                there will be an id appenden out of millis() + random(int) to use multiple requests the same time by the id you can difference it e.g if you want multiple devices for the same service (implemented later)

        */
        String formatComMessage(bool request = false, bool generateId = false, String serviceName = "n.S", String MAC = "n.S", String ip = "n.S", String port = "-1", String id = "n.S");

        /*
            return the correct Filename to use with LittleFS - only helper function

            if fallback = false
                mainCFG File of ServiceName will returned
            else
                FallbackCFG File will returned 

            This function doesn't check for existence of the specified File - it only creates the correct path
        */
        String getExternalServiceFilename(const char* serviceName, bool fallback = false);

        //create the "Basic Internal offered Services File " - content is only this as a Service (NetworkIdent@Port:63547)
        bool createInternalServicesBasicConfigFile();

        //added in loop to handle incoming messages
        void handleRequests(); 

        ulong getLastGeneratedId();
        StaticJsonDocument<425> getLastData();


    public:
        //Constructo / Destructor
        ServiceHandler(Filemanager* FM, WiFiManager* wifiManager);
        ~ServiceHandler();

        //Basic Control
        bool beginListen();
        void stopListen();


        //serviceManagement
        /*
            addService
                if selfOffered = true
                    only serviceName and port need to be added - other parameter will ignored
                else
                    all parameters are needed

                DEV-INF:
                    Auto assign MAC Address need to be added! - V1.2
        */
        bool addService(bool selfOffered = true, bool fallback = false, const char* serviceName = "n.S", const char* port = "-1", IPAddress ip = IPAddress(0,0,0,0));

        /*
            Search in Network for other devices - saved <<serviceName>> and try add CFG for this service
            -> only external Services can be added this way - but i think i don't need to explain here why^^


            res:
                0 = no Networkdevice found with specified service (no device in network or no connection)
                1 = device found and added (success)
                2 = device found but service can't be added
        */
        short autoAddService(const char* serviceName);



        /*
            delService
                if selfOffered = true
                    only serviceName is needed - other params will be ignored
                else
                    if fallback = true
                        only the fallback CFG of external Service will be deleted
                    else
                        only the mainCFG of external Serivce will be deleted
        */
        bool delService(const char* serviceName, bool selfOffered = false, bool fallback = false);


        //get Stuff
        //  for internal useage to get Address and Port of specified external Services

        /*
            getServiceIP
                on success: return IPAddress
                on fail: return IP(0,0,0,0)
        */
        IPAddress getServiceIP(const char* serviceName, bool fallback = false);

        /*
            getServiceMAC
                onSuccess: return MAC
                onFail: return "failed"
        */
        String getServiceMAC(const char* serviceName, bool fallback = false);

        /*
            getServicePort
                onSuccess: return port
                onFail: return -1
        */
        int getServicePort(const char* serviceName, bool fallback = false);
        
        /*
            CheckForService
                res = 0 -> service doesn't exist
                res = 1 -> internal Service (self offered)
                res = 2 -> external Service
                res = 3 -> found as internal and external
        */
        short checkForService(const char* serviceName, bool onlyExternal = false);

        /*
            changeConfigValue
                changes an existing Config and the specified Value to <<toChangeKey>>
                if changeInternalService "true"
                    an internalService will be changed - <<changeFallback>> will be ignored
                if changeInternalService "false"
                    an existing external Service will changed e.g for manual override - 
                        if <<changeFallback>> "true"
                            -> the fallback Config will be changed
                        else
                            -> the main Config will be changed

                if the File is not existing, the key not 
        */
        bool changeConfigValue(const char* serviceName, const char* toChangeKey, const char* newValue, bool changeInternalService = true, bool changeFallback = false);

        /*
            Search for Service
        */
       void searchForService(const char* serviceName, bool generateId = true, IPAddress ip = IPAddress(255,255,255,255), int port = 63547);

        //loop
        void loop();

        /*
        Inherited ErrorHandling

        These are very Basic implementations and will be fixed / better implemented later
       */
        void startClass();
        void stopClass();
        void pauseClass();
        void restartClass();
        void continueClass();
};

#endif