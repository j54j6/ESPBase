#pragma once
#ifndef J54J6_OTA
#define J54J6_OTA

#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#include <StreamString.h>

#include "filemanager.h"
#include "moduleState.h"
#include "logger.h"
#include "network.h"
#include "ntpManager.h"
#include "wifiManager.h"

/*
    Update Process:
        Client->Server - request
        -> Header:
                - x-requestType ('updateCheck')
                - x-usedChip ('FM->readChip')
                - x-softwareVersion ('FM->readSoftwareVersion')
                - x-PasswordAuth ('FM->readPW) (Added automatic)
                - x-updateToken ('FM->readToken) (Added automatic)

        Response:
        Server->Client - response
        -> Header:
            -> HTTP Code 200
            - x-requestedType ('updateCheck')
            - x-requestResponse ('response Message') 
                -> ("makeUpdate" (forceUpdate for Important stuff)) 
                -> ("newVersionAvail" (notify for new Version and device decide what to do)) 
                -> ("noUpdates" (no Updates availiable))
            - x-versionAvailiable (actualVersion number availiable)
        
        Server->Client - response
            -> HTTP Code 200
            -> x-requestedType ('update')
            -> x-requestResponse('installUpdate) //start Update
            -> x-versionAvailiable (version of database)
*/


/*
    Default Values for config

*/
#define defaultUpdateServer "192.168.178.27"
#define defaultUpdateUrl "/"
#define defaultUpdateToken "espWiFiThermometerV1"
#define defaultUpdatePass "rtgzi32u45z4u5hbnfdnfbdsi"
#define defaultUpdatePort "80"
#define defaultUpdateUpdateSearch "true"
#define defaultAutoUpdate "true"
#define defaultUpdateSoftwareVeresion "0.0.1"
#define defaultUpdateCheckDelay "86400"




class OTA_Manager
{
    private:
        const char* configFile = "config/mainConfig.json";
        Filemanager* _FM;
        Network* _Network;
        WiFiManager* _Wifi;
        NTPManager* _Ntp;
        LED* _updateLed;
        SysLogger logging;
        ClassModuleSlave classControl = ClassModuleSlave("OTA", 1);

        bool afterUpdateCheck = false; //used to take actions after update - in configFile is an 
        bool updatesAvailiable = false;
        bool autoUpdate = true;
        long lastUpdateCheck = 0;
        long checkIntervall = 0;
        ulong nextUpdateCheck = 0;
        bool automaticUpdateSearch = true;
        String softwareVersionInstalled = "0";
        String softwareVersionAvailiable = "0";
        bool lastFailed = false;
        bool initCorrect = false;

        /*
            functionType:
                1 -> updateCheck
                2 -> update
        */
        bool addHeader(HTTPClient* client, int functionType = 1);

        //debug function
        void showHeader(HTTPClient* http)
        {
            /*
            Serial.println("###########header START#################");
            for(int i = 0; i < http->headers(); i++)
            {
                Serial.println(http->headerName(i) + String(" : ") + http->header(i));
            }
            Serial.println("##############END HEADER###############");
            */
        }
    protected:
        bool checkForFiles();
    public:
        OTA_Manager(){};
        OTA_Manager(Filemanager* FM, Network* network, NTPManager* ntp, WiFiManager* _Wifi, LED* updateLed = new LED());
        bool init();
        bool checkForUpdates(String host, uint16_t port, String uri, String username, String password, bool onlyCheck = false);
        bool getUpdates(String host, uint16_t port, String uri, String username, String password);
        bool installUpdate(Stream& in, uint32_t size, const String& md5, int command);
        
        bool checkUpdatesAutoCred()
        {
            int port = String(_FM->readJsonFileValue("config/mainConfig.json", "port")).toInt();
            return checkForUpdates(_FM->readJsonFileValue(configFile, "updateServer"),
            port, _FM->readJsonFileValue(configFile, "uri"),
            _FM->readJsonFileValue(configFile, "servertoken"), _FM->readJsonFileValue(configFile, "serverpass"));
        }
        bool getUpdatesAutoCred()
        {
            int port = String(_FM->readJsonFileValue("config/mainConfig.json", "port")).toInt();
            return getUpdates(_FM->readJsonFileValue(configFile, "updateServer"),
            port, _FM->readJsonFileValue(configFile, "uri"),
            _FM->readJsonFileValue(configFile, "servertoken"), _FM->readJsonFileValue(configFile, "serverpass"));
        }

        bool setAutoUpdate(bool autoUpdate = true);
        bool setSearchForUpdatesAutomatic(bool search = true);
        bool setUpdateServer(String host, uint16_t port = 80, String uri = "/");
        bool setCheckIntervall(long checkDelay = 24); //in hours
        bool setServerToken(String newToken = "username");
        bool setServerPass(String newPass = "newPass");
        
        bool getAutoUpdate();
        bool getSearchForUpdatesAutomatic();
        long getCheckIntervall();
        long getLastUpdateCheck();
        bool getIsLastUpdateCheckFailed();
        bool getIsUpdateAvailiable();
        void run();

};
#endif