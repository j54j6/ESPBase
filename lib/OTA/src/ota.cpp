#include "ota.h"

OTA_Manager::OTA_Manager(Filemanager* FM, Network* network, NTPManager* ntp, WiFiManager* _Wifi, LED* updateLed) {
    this->_FM = FM;
    this->_Network = network;
    this->_Ntp = ntp;
    this->_Wifi = _Wifi;
    this->_updateLed = updateLed;
    this->logging = SysLogger(_FM, "OTA-Manager");
    init();
}

bool OTA_Manager::checkForFiles() 
{
    if(!_FM->fExist(configFile))
    {
        logging.logIt("checkForFiles", "createConfigFile", 2);
        _FM->createFile(configFile);
        _FM->appendJsonKey(configFile, "updateServer", "update.nodework.de");
    }
    else
    {
        logging.logIt("checkForFiles", "Config File already exist -SKIP");
    }
}

bool OTA_Manager::init()
{
    this->automaticUpdateSearch = _FM->returnAsBool(_FM->readJsonFileValue(configFile, "updateSearch"));
    this->checkIntervall = String(_FM->readJsonFileValue(configFile, "checkDelay")).toFloat();
    this->lastUpdateCheck = String(_FM->readJsonFileValue(configFile, "lastUpdateSearch")).toFloat();
    this->autoUpdate = _FM->returnAsBool(_FM->readJsonFileValue(configFile, "autoUpdate"));
    this->nextUpdateCheck = lastUpdateCheck + checkIntervall;

    this->softwareVersionInstalled = _FM->readJsonFileValue(configFile, "softwareVersion");
    this->softwareVersionAvailiable = _FM->readJsonFileValue(configFile, "softwareVersionAvail");

    logging.logIt("init", "Automatic update Search: " + String(automaticUpdateSearch));
    logging.logIt("init", "Checkintervall: " + String(checkIntervall));
    logging.logIt("init", "last Update Check: " + String(lastUpdateCheck));
    double nextCheck = (nextUpdateCheck - _Ntp->getEpochTime());
    nextCheck = nextCheck / 3600;
    logging.logIt("init", "Next Update Check: " + String(nextCheck) + String("h"));
    logging.logIt("init", "Firmwareversion: " + String(softwareVersionInstalled));
    logging.logIt("init", "Firmwareversion Availiable: " + String(softwareVersionAvailiable));
    logging.logIt("init", "Auto Update: " + String(autoUpdate));
    return true;
}

bool OTA_Manager::addHeader(HTTPClient* client, int functionType)
{
    client->setUserAgent(F("Nodework-Updater-Class"));
    client->addHeader(F("x-Chip-ID"), String(ESP.getChipId()));
    client->addHeader(F("x-STA-MAC"), WiFi.macAddress());
    client->addHeader(F("x-AP-MAC"), WiFi.softAPmacAddress());
    client->addHeader(F("x-free-space"), String(ESP.getFreeSketchSpace()));
    client->addHeader(F("x-sketch-size"), String(ESP.getSketchSize()));
    client->addHeader(F("x-sketch-md5"), String(ESP.getSketchMD5()));
    client->addHeader(F("x-chip-size"), String(ESP.getFlashChipRealSize()));
    client->addHeader(F("x-sdk-version"), ESP.getSdkVersion());
    client->addHeader(F("x-mode"), F("sketch"));
    client->addHeader(F("x-usedChip"), String(_FM->readJsonFileValue(configFile, "usedChip"))); //add Hardware type to get correct firmware
    client->addHeader(F("x-software-Version"), String(_FM->readJsonFileValue(configFile, "softwareVersion"))); //add header to get Correct update state
    switch(functionType)
    {
        case 1:
            client->addHeader(F("x-requestType"), String("updateCheck")); //add header to trigger update check
            break;
        case 2:
            client->addHeader(F("x-requestType"), String("update")); //add header to trigger update check
            break;
        case 3:
            client->addHeader(F("x-requestType"), String("undefined")); //add header to trigger update check
            break;
    }
}



bool OTA_Manager::checkForUpdates(String host, uint16_t port, String uri, String username, String password, bool onlyCheck)
{
    checkForFiles();
    if(!_Wifi->isConnected()) //if no network Conenction return false don't create any Objects or whatever
    {
        logging.logIt("checkForUpdates", "Can't check for Updates - no Network Connection!", 4);
        return false;
    }

    HTTPClient http; //create HTP Object to communicate with a webserver
    logging.logIt("checkForUpdates", "Update-Server: " + String(host) + String(" Port: ") + String(port) + String(" Uri: ") + String(uri));
    logging.logIt("checkForUpdates", "Username: " + String(username) + String(", password: ") + String(password), 2);

    http.begin(host, port, uri); //change with HTTPClient::begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint) - start coounication
    addHeader(&http); //add basic device header

    const char * headerkeys[] = { "x-MD5", "x-requestedType", "x-requestResponse", "x-versionAvailiable"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

    // track these headers
    http.collectHeaders(headerkeys, headerkeyssize);
    http.setAuthorization(username.c_str(), password.c_str()); //read server credentials from Flash

    int code = http.GET(); 
    int len = http.getSize();

    showHeader(&http);

    if(code <= 0)
    {
        logging.logIt("checkForUdates", String("Error while checking for updates! - Code: ") + String(code) + String("Error: ") + String(http.errorToString(code).c_str()), 4);
        return false;
    }
    else if(!http.hasHeader("x-requestedType") || !http.hasHeader("x-requestResponse"))
    {
        logging.logIt("checkForUdates", String("Error while checking for updates! - HTTP Response is invalid! - HTTP Code: " + String(code)), 4);
        return false;
    }
    else if(http.header("x-requestedType") != String("updateCheck"))
    {
        logging.logIt("checkForUdates", String("Error while checking for updates! - HTTP Response has an unexcepted value"), 4);
        return false;
    }

    if(http.header("x-requestedType") = String("updateCheck"))
    {
        if(code != 200)
        {
            logging.logIt("checkForUdates", String("Error while checking for updates! - HTTP return unexcpeted Code: ") + String(code) + String("\n ErrorMessage: ") + String(http.errorToString(code)));
            return false;
        }
        else
        {
            _FM->changeJsonValueFile(configFile, "lastUpdateSearch", String(_Ntp->getEpochTime()).c_str());
            _FM->changeJsonValueFile(configFile, "softwareVersionAvail", String(http.header("x-versionAvailiable")).c_str());
            //markerHere
            Serial.println("------Update Check response get");
            showHeader(&http);
            if(http.header("x-requestResponse") == String("makeUpdate")) //device is forced to Update - only used for important update fixes like security
            {
                logging.logIt("checkForUpdates", "Updates found. Current Version: " + String(_FM->readJsonFileValue(configFile, "softwareVersion")) + String(" New Version: ") + String(http.header("x-newVersion")));
                logging.logIt("checkForUpdates", "Device is forced by Server to update! -> run Update");
                updatesAvailiable = true;
                if(onlyCheck)
                {
                    return true;
                }
                else
                {
                    return getUpdates(host, port, uri, username, password);
                }
            }
            else if(http.header("x-requestResponse") == String("newVersionAvail")) //device can be updated but is not forced to - device/user settings decide if device get's the update
            {
                logging.logIt("checkForUpdates", "Updates found. Current Version: " + String(_FM->readJsonFileValue(configFile, "softwareVersion")) + String(" New Version: ") + String(http.header("x-newVersion")));
                if(_FM->returnAsBool(_FM->readJsonFileValue(configFile, "autoUpdate")) && !onlyCheck) //device get auto Updates
                {
                    return getUpdates(host, port, uri, username, password);
                }
                else
                {
                    updatesAvailiable = true;
                    _FM->changeJsonValueFile(configFile, "lastUpdateSearch", String(_Ntp->getEpochTime()).c_str());
                    return true;
                }
            }   
        }
    }
    return false;
}

//getUpdates(_FM->readJsonFileValue(configFile, "updateServer"), uint(_FM->readJsonFileValue(configFile, "port")), _FM->readJsonFileValue(configFile, "uri"), _FM->readJsonFileValue(configFile, "servertoken"), _FM->readJsonFileValue(configFile, "serverpass"));
bool OTA_Manager::getUpdates(String host, uint16_t port, String uri, String username, String password)
{
    _updateLed->blink(300, false, 20);
    logging.logIt("getUpdates", "Start firmware Update", 3);

/*
    if(!checkForUpdates(host, port, uri, username, password, true))
    {
        logging.logIt("getUpdates", "No Updates availiable! - return", 3);
        return true;
    }
    */
    HTTPClient http; //create HTP Object to communicate with a webserver
    
    http.begin(host, port, uri); //change with HTTPClient::begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint) - start coounication
    addHeader(&http, 2); //add basic device header

    const char * headerkeys[] = { "x-MD5", "x-requestedType", "x-requestResponse", "x-versionAvailiable"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    // track these headers
    http.collectHeaders(headerkeys, headerkeyssize);
    http.setAuthorization(username.c_str(), password.c_str()); //read server credentials from Flash

    int code = http.GET(); 
    int len = http.getSize();
    logging.logIt("checkForUpdates", String("HTTP Code: ") + String(code), 2);

    if(len > (int) ESP.getFreeSketchSpace()) {
        logging.logIt("getUpdates", "Not enough memory availiable to run the Update! Needed: " + String(len) + String(" bytes, Availiable: ") + String(ESP.getFreeSketchSpace()));
        classControl.newReport("Can't Update device - not enough memory!", 2421, 2, true);
        return false;
    }

    //warn state implemented later
    WiFiClient *tcp = http.getStreamPtr();

    //WiFiUDP::stopAll(); //later implemented by cconfig
    //WiFiClient::stopAllExcept(tcp);
    uint8_t buf[4];
    if(tcp->peekBytes(&buf[0], 4) != 4) {
        logging.logIt("getUpdate", "peakBytes didn't find magic header! - Update failed!", 5);
        http.end();
        return false;
    }

    // check for valid first magic byte
    if(buf[0] != 0xE9 && buf[0] != 0x1f) 
    {
        logging.logIt("getUpdate", "Magic header does not start with 0xE9 - Update failed!", 4);
        http.end();
        return false;
    }
    if (buf[0] == 0xe9) 
    {
        uint32_t bin_flash_size = ESP.magicFlashChipSize((buf[3] & 0xf0) >> 4);
        // check if new bin fits to SPI flash
        if(bin_flash_size > ESP.getFlashChipRealSize()) {
            logging.logIt("getUpdate", "New binary does not fit SPI Flash size - Update failed!", 4);
            http.end();
            return false;
        }
    }
    Serial.println("-------getUpdate end");
    showHeader(&http);
    if(installUpdate(*tcp, len, http.header("x-MD5"), 0)) 
    {
        logging.logIt("getUpdate", "Update Successful - Save new Version");

        _FM->changeJsonValueFile(configFile, "softwareVersion", String(http.header("x-versionAvailiable")).c_str());
        //later implemented state set update done - later with (rebootOn Update config)
        ESP.restart();
    }
    else 
    {
        logging.logIt("getUpdate", "Update failed! - Installation failed!", 5);
        return false;
    }
}

/**
 * write Update to flash
 * @param in Stream&
 * @param size uint32_t
 * @param md5 String
 * @return true if Update ok
 */
bool OTA_Manager::installUpdate(Stream& in, uint32_t size, const String& md5, int command)
{
    StreamString error;

    if(!Update.begin(size, command)) {
        Update.printError(error);
        error.trim(); // remove line ending
        logging.logIt("installUpdate", "Error - Update.begin failed!" + String(error.c_str()), 5);
        return false;
    }
    if(md5.length()) {
        if(!Update.setMD5(md5.c_str())) {
            logging.logIt("installUpdate", "Error - Update.setMD5 failed!" + String(md5.c_str()), 5);
            return false;
        }
    }
    if(Update.writeStream(in) != size) {
        Update.printError(error);
        error.trim(); // remove line ending
        logging.logIt("installUpdate", "Error - Update.writeStream failed!" + String(error.c_str()), 5);
        return false;
    }
    if(!Update.end()) {
        Update.printError(error);
        error.trim(); // remove line ending
        logging.logIt("installUpdate", "Error - Update.end failed!", 5);
        return false;
    }
    return true;
}


bool OTA_Manager::setAutoUpdate(bool autoUpdate)
{
    if(autoUpdate)
    {
       return _FM->changeJsonValueFile(configFile, "autoUpdate", "true"); 
    }
    return _FM->changeJsonValueFile(configFile, "autoUpdate", "false");
}

bool OTA_Manager::setSearchForUpdatesAutomatic(bool search)
{
    if(search)
    {
       return _FM->changeJsonValueFile(configFile, "updateSearch", "true"); 
    }
    return _FM->changeJsonValueFile(configFile, "updateSearch", "false");
}

bool OTA_Manager::setUpdateServer(String host, uint16_t port, String uri)
{
    bool hostChange = _FM->changeJsonValueFile(configFile, "updateServer", host.c_str());
    bool portChange = _FM->changeJsonValueFile(configFile, "port", String(port).c_str());
    bool uriChange = _FM->changeJsonValueFile(configFile, "uri", uri.c_str());

    if(hostChange & portChange & uriChange)
    {
        return true;
    }
    return false;
}

bool OTA_Manager::setCheckIntervall(long checkDelay)
{
    long checkDelayInSecods = checkDelay * 3600;
    return _FM->changeJsonValueFile(configFile, "checkDelay", String(checkDelayInSecods).c_str());
}

bool OTA_Manager::setServerToken(String newToken)
{
    return _FM->changeJsonValueFile(configFile, "servertoken", newToken.c_str());
}

bool OTA_Manager::setServerPass(String newPass)
{
    return _FM->changeJsonValueFile(configFile, "serverpass", newPass.c_str());
}

bool OTA_Manager::getAutoUpdate()
{
    return _FM->returnAsBool(_FM->readJsonFileValue(configFile, "autoUpdate"));
}

bool OTA_Manager::getSearchForUpdatesAutomatic()
{
    return _FM->returnAsBool(_FM->readJsonFileValue(configFile, "updateSearch"));
}

long OTA_Manager::getCheckIntervall()
{
    long checkIntervallInSeconds = long(_FM->readJsonFileValue(configFile, "checkDelay"));
    return checkIntervallInSeconds/3600;
}

long OTA_Manager::getLastUpdateCheck()
{
    long lastCheck = long(_FM->readJsonFileValue(configFile, "lastUpdateSearch"));
    return lastCheck;
}

void OTA_Manager::run()
{
    static int reInit = 0;

    _Ntp->run();
    _Network->run();
    _Wifi->run();

    //check for updateCheck
    if(automaticUpdateSearch)
    {
        if(_Ntp->getEpochTime() > nextUpdateCheck)
        {
            int port = String(_FM->readJsonFileValue("config/mainConfig.json", "port")).toInt();
            bool res = this->checkForUpdates(_FM->readJsonFileValue(configFile, "updateServer"), port, _FM->readJsonFileValue(configFile, "uri"),
            _FM->readJsonFileValue(configFile, "servertoken"), _FM->readJsonFileValue(configFile, "serverpass"));

            if(!res)
            {
                logging.logIt("run->checkForUpdates", "Check for Updates return false!");
                init();
            }
        }
    }

    if(millis() > reInit)
    {
        reInit = millis() + 3600000;
        init();
    }
}