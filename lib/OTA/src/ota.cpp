#include "ota.h"

OTA_Manager::OTA_Manager(Filemanager* FM, Network* network) {
    this->_FM = FM;
    this->_Network = network;
    checkForFiles();
}

bool OTA_Manager::checkForFiles() 
{
    if(!_FM->fExist(configFile))
    {
        logging.logIt("checkForFiles", "createConfigFile", 2);
        _FM->createFile(configFile);
        _FM->appendJsonKey(configFile, "updateServer", "update.nodework.de");
    }
}


bool OTA_Manager::addHeader(HTTPClient* client)
{
    client->setUserAgent(F("Nodework-Updater-Class"));
    client->addHeader(F("x-ESP8266-Chip-ID"), String(ESP.getChipId()));
    client->addHeader(F("x-ESP8266-STA-MAC"), WiFi.macAddress());
    client->addHeader(F("x-ESP8266-AP-MAC"), WiFi.softAPmacAddress());
    client->addHeader(F("x-ESP8266-free-space"), String(ESP.getFreeSketchSpace()));
    client->addHeader(F("x-ESP8266-sketch-size"), String(ESP.getSketchSize()));
    client->addHeader(F("x-ESP8266-sketch-md5"), String(ESP.getSketchMD5()));
    client->addHeader(F("x-ESP8266-chip-size"), String(ESP.getFlashChipRealSize()));
    client->addHeader(F("x-ESP8266-sdk-version"), ESP.getSdkVersion());
    client->addHeader(F("x-ESP8266-mode"), F("sketch"));
}

bool OTA_Manager::checkForUpdates(String host, uint16_t port, String uri, String username, String password)
{
    HTTPClient http;
    
    http.begin(host, port, uri);
    addHeader(&http);
    http.addHeader(F("x-ESP8266-sketch-size"), String("updateCheck"));
    const char * headerkeys[] = { "x-MD5", "x-requestedType", "x-requestResponse"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

    // track these headers
    http.collectHeaders(headerkeys, headerkeyssize);
    http.setAuthorization("espWiFiThermometerV1", "rtgzi32u45z4u5hbnfdnfbdsi");

    int code = http.GET();
    int len = http.getSize();
    logging.logIt("checkForUpdates", String("HTTP Code: ") + String(code));
    logging.logIt("checkForUpdates", String("Header Size: ") + String(len));
    logging.logIt("checkForUpdates", String("Content: ") + String(http.getString()));
    logging.logIt("checkForUpdates", String("Error: ") + String(http.errorToString(code)));

    if(code <= 0)
    {
        logging.logIt("checkForUdates", String("Error while checking for updates! - Code: ") + String(code) + String("Error: ") + String(http.errorToString(code).c_str()));
        return false;
    }
    else if(http.hasHeader("x-requestedType") && http.hasHeader("x-requestResponse"))
    {
        logging.logIt("checkForUdates", String("Error while checking for updates! - HTTP Response is invalid!"));
        return false;
    }
    else if(http.header("x-requestedType") != String("updateCheck"))
    {
        logging.logIt("checkForUdates", String("Error while checking for updates! - HTTP Response has an unexcepted value"));
        return false;
    }

    if(http.header("x-requestedType") = String("updateCheck"))
    {
        
    }
}