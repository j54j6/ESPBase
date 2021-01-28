#ifndef J54J6_OTA
#define J54J6_OTA

#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>

#include "filemanager.h"
#include "moduleState.h"
#include "logger.h"
#include "network.h"


class OTA_Manager
{
    private:
        const char* configFile = "/config/mainConfig.json";
        Filemanager* _FM;
        Network* _Network;
        SysLogger logging;

        bool addHeader(HTTPClient* client);

    protected:
        bool checkForFiles();
    public:
        OTA_Manager(Filemanager* FM, Network* network);
        bool checkForUpdates(String host, uint16_t port, String uri, String username, String password);           
};
#endif