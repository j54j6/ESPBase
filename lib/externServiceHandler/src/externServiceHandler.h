#ifndef J54J6_SERVICEHANDLER_H
#define J54J6_SERVICEHANDLER_H

#include "errorHandler.h"
#include "NetworkIdent.h"
#include "wifiManager.h"
#include "filemanager.h"

class ExternServiceHandler : public ErrorSlave {
    private:
        const char* className = "ExternServiceHandler";
        bool classDisabled = false;
        const char* serviceAddressListFile = "/config/serviceHandler/registered/services.json";

        //handler
        Filemanager* FM;
        WiFiManager* wifiManager;
        NetworkIdent* networkIdent;

    public:
        ExternServiceHandler(Filemanager* FM, WiFiManager* wifiManager, NetworkIdent* networkIdent);

        int getServiceDestPort(const char* serviceName); //look in configFile for preDefined Service Port if nothing found autoAdd is used
        IPAddress getServiceIP(const char* serviceName); //look in configFile for preDefined Service Address if nothing found autoAdd is used
        bool autoAddService(const char* serviceName); //search and add service to config if found in network
        bool manAddService(const char* serviceName, int port, IPAddress ip, bool checkConnect = true, const char* MAC = "n.S"); //manually write Service into File e.g for webInterface 
        bool delService(const char* serviceName);


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