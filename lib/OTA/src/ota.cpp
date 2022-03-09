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
        logging.logIt(F("checkForFiles"), F("createConfigFile"), 2);
        _FM->createFile(configFile);
        return _FM->appendJsonKey(configFile, "updateServer", "update.nodework.de");
    }
    else
    {
        bool cupdateServer = _FM->checkForKeyInJSONFile(configFile, "updateServer");
        bool cupdateuri = _FM->checkForKeyInJSONFile(configFile, "uri");
        bool cservertoken = _FM->checkForKeyInJSONFile(configFile, "servertoken");
        bool cserverpass = _FM->checkForKeyInJSONFile(configFile, "serverpass");
        bool cport = _FM->checkForKeyInJSONFile(configFile, "port");
        bool cupdateSearch = _FM->checkForKeyInJSONFile(configFile, "updateSearch");
        bool cautoUpdate = _FM->checkForKeyInJSONFile(configFile, "autoUpdate");
        bool cSoftwareversion = _FM->checkForKeyInJSONFile(configFile, "softwareVersion");
        bool cCheckDelay =  _FM->checkForKeyInJSONFile(configFile, "checkDelay");

        static bool changed = false;
        if(!cupdateServer)
        {
            _FM->appendJsonKey(configFile, "updateServer", defaultUpdateServer);
            changed = true;
        }
        if(!cupdateuri)
        {
            _FM->appendJsonKey(configFile, "uri", defaultUpdateUrl);
            changed = true;
        }
        if(!cservertoken)
        {
            _FM->appendJsonKey(configFile, "servertoken", defaultUpdateToken);
            changed = true;
        }
        if(!cserverpass)
        {
            _FM->appendJsonKey(configFile, "serverpass", defaultUpdatePass);
            changed = true;
        }
        if(!cport)
        {
            _FM->appendJsonKey(configFile, "port", defaultUpdatePort);
            changed = true;
        }
        if(!cupdateSearch)
        {
            _FM->appendJsonKey(configFile, "updateSearch", defaultUpdateUpdateSearch);
            changed = true;
        }
        if(!cautoUpdate)
        {
            _FM->appendJsonKey(configFile, "autoUpdate", defaultAutoUpdate);
            changed = true;
        }
        if(!cSoftwareversion)
        {
            _FM->appendJsonKey(configFile, "softwareVersion", defaultUpdateSoftwareVeresion);
            changed = true;
        }
        if(!cCheckDelay)
        {
            _FM->appendJsonKey(configFile, "checkDelay", defaultUpdateCheckDelay);
            changed = true;
        }

        if(changed)
        {
            logging.logIt(F("checkForFiles"), F("Config File was not complete! - check again!"));
            bool res = checkForFiles();

            if(res)
            {
                logging.logIt(F("checkForFiles"), F("Config File complete! - return true"), 2);
                changed = false;
                return true;
            }
            else
            {
                logging.logIt(F("checkForFiles"), F("Config File can't be completed! - FAILED"), 4);
                return false;
            }
        }
        logging.logIt(F("checkForFiles"), F("Config File complete -SKIP"));
        return true;
    }
    return false;
}

bool OTA_Manager::init()
{
    if(!this->_Wifi->isConnected())
    {
        logging.logIt(F("init"), F("Can't init - WiFI is not connected"));
        return false;
    }
    if(!_Ntp->updateTime())
    {
        logging.logIt(F("init"), F("Can't update Time!"));
        return false;
    }
    checkForFiles();
    this->automaticUpdateSearch = _FM->returnAsBool(_FM->readJsonFileValue(configFile, "updateSearch"));
    this->checkIntervall = String(_FM->readJsonFileValue(configFile, "checkDelay")).toFloat();
    this->lastUpdateCheck = String(_FM->readJsonFileValue(configFile, "lastUpdateSearch")).toFloat();
    this->autoUpdate = _FM->returnAsBool(_FM->readJsonFileValue(configFile, "autoUpdate"));
    this->nextUpdateCheck = lastUpdateCheck + checkIntervall;

    this->softwareVersionInstalled = _FM->readJsonFileValue(configFile, "softwareVersion");
    this->softwareVersionAvailiable = _FM->readJsonFileValue(configFile, "softwareVersionAvail");

    logging.logIt(F("init"), "Automatic update Search: " + String(automaticUpdateSearch));
    logging.logIt(F("init"), "Checkintervall: " + String(checkIntervall));
    logging.logIt(F("init"), "last Update Check: " + String(lastUpdateCheck));
    double nextCheck = (nextUpdateCheck - _Ntp->getEpochTime());
    nextCheck = nextCheck / 3600;
    logging.logIt(F("init"), "Next Update Check: " + String(nextCheck) + String("h"));
    logging.logIt(F("init"), "Firmwareversion: " + String(softwareVersionInstalled));
    logging.logIt(F("init"), "Firmwareversion Availiable: " + String(softwareVersionAvailiable));
    logging.logIt(F("init"), "Auto Update: " + String(autoUpdate));
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
    client->addHeader(F("x-fs-Version"), String(_FM->readJsonFileValue(configFile, "fsVersion"))); //add header to get Correct update FS state
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
        case 4:
            client->addHeader(F("x-requestType"), String("filesystemCheck")); //add header to trigger filesystemCheck
            break;
        case 5:
            client->addHeader(F("x-requestType"), String("filesystemUpdate")); //add header to trigger filesystemUpdate
            break;
    }
    return true;
}



bool OTA_Manager::checkForUpdates(String host, uint16_t port, String uri, String username, String password, bool onlyCheck)
{
    checkForFiles();
    if(!_Wifi->isConnected()) //if no network Conenction return false don't create any Objects or whatever
    {
        logging.logIt(F("checkForUpdates"), F("Can't check for Updates - no Network Connection!"), 4);
        return false;
    }

    HTTPClient http; //create HTP Object to communicate with a webserver
    logging.logIt(F("checkForUpdates"), "Update-Server: " + String(host) + String(" Port: ") + String(port) + String(" Uri: ") + String(uri));
    logging.logIt(F("checkForUpdates"), "Username: " + String(username) + String(", password: ") + String(password), 2);

    //bool begin(WiFiClient &client, const String& host, uint16_t port, const String& uri = "/", bool https = false);
    http.begin(*_Wifi->getWiFiClient(), host, port, uri); //change with HTTPClient::begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint) - start coounication
    addHeader(&http); //add basic device header

    const char * headerkeys[] = { "x-MD5", "x-requestedType", "x-requestResponse", "x-versionAvailiable"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

    // track these headers
    http.collectHeaders(headerkeys, headerkeyssize);
    http.setAuthorization(username.c_str(), password.c_str()); //read server credentials from Flash

    int code = http.GET(); 
    //int len = http.getSize();

    showHeader(&http);

    if(code <= 0)
    {
        logging.logIt(F("checkForUdates"), String("Error while checking for updates! - Code: ") + String(code) + String("Error: ") + String(http.errorToString(code).c_str()), 4);
        this->nextUpdateCheck = _Ntp->getEpochTime() + 3600;
        return false;
    }
    else if(!http.hasHeader("x-requestedType") || !http.hasHeader("x-requestResponse"))
    {
        logging.logIt(F("checkForUdates"), String("Error while checking for updates! - HTTP Response is invalid! - HTTP Code: " + String(code)), 4);
        this->nextUpdateCheck = _Ntp->getEpochTime() + 3600;
        Serial.println(nextUpdateCheck);
        return false;
    }
    else if(http.header("x-requestedType") != String("updateCheck"))
    {
        logging.logIt(F("checkForUdates"), String("Error while checking for updates! - HTTP Response has an unexcepted value"), 4);
        this->nextUpdateCheck = _Ntp->getEpochTime() + 3600;
        return false;
    }

    if(http.header("x-requestedType") == String("updateCheck"))
    {
        if(code != 200)
        {
            logging.logIt(F("checkForUdates"), String("Error while checking for updates! - HTTP return unexcpeted Code: ") + String(code) + String("\n ErrorMessage: ") + String(http.errorToString(code)));
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
                logging.logIt(F("checkForUpdates"), "Updates found. Current Version: " + String(_FM->readJsonFileValue(configFile, "softwareVersion")) + String(" New Version: ") + String(http.header("x-newVersion")));
                logging.logIt(F("checkForUpdates"), "Device is forced by Server to update! -> run Update");
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
                logging.logIt(F("checkForUpdates"), "Updates found. Current Version: " + String(_FM->readJsonFileValue(configFile, "softwareVersion")) + String(" New Version: ") + String(http.header("x-versionAvailiable")));
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
            else if(http.header("x-requestResponse") == String("noUpdates"))
            {
                logging.logIt(F("checkForUpdates"), F("No Updates"));
                return true;
            }
        }
    }
    return false;
}

//getUpdates(_FM->readJsonFileValue(configFile, "updateServer"), uint(_FM->readJsonFileValue(configFile, "port")), _FM->readJsonFileValue(configFile, "uri"), _FM->readJsonFileValue(configFile, "servertoken"), _FM->readJsonFileValue(configFile, "serverpass"));
bool OTA_Manager::getUpdates(String host, uint16_t port, String uri, String username, String password)
{
    _updateLed->blink(300, false, 20);
    logging.logIt(F("getUpdates"), F("Start firmware Update"), 3);

/*
    if(!checkForUpdates(host, port, uri, username, password, true))
    {
        logging.logIt(F("getUpdates", "No Updates availiable! - return", 3);
        return true;
    }
    */
    HTTPClient http; //create HTP Object to communicate with a webserver
    
    http.begin(*_Wifi->getWiFiClient(), host, port, uri); //change with HTTPClient::begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint) - start coounication
    addHeader(&http, 2); //add basic device header

    const char * headerkeys[] = { "x-MD5", "x-requestedType", "x-requestResponse", "x-versionAvailiable"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    // track these headers
    http.collectHeaders(headerkeys, headerkeyssize);
    http.setAuthorization(username.c_str(), password.c_str()); //read server credentials from Flash

    int code = http.GET(); 
    int len = http.getSize();
    logging.logIt(F("checkForUpdates"), String("HTTP Code: ") + String(code), 2);

    if(len > (int) ESP.getFreeSketchSpace()) {
        logging.logIt(F("getUpdates"), "Not enough memory availiable to run the Update! Needed: " + String(len) + String(" bytes, Availiable: ") + String(ESP.getFreeSketchSpace()));
        classControl.newReport(F("Can't Update device - not enough memory!"), 2421, 2, true);
        return false;
    }

    //warn state implemented later
    WiFiClient *tcp = http.getStreamPtr();

    //WiFiUDP::stopAll(); //later implemented by cconfig
    //WiFiClient::stopAllExcept(tcp);
    uint8_t buf[4];
    if(tcp->peekBytes(&buf[0], 4) != 4) {
        logging.logIt(F("getUpdate"), F("peakBytes didn't find magic header! - Update failed!"), 5);
        http.end();
        return false;
    }

    // check for valid first magic byte
    if(buf[0] != 0xE9 && buf[0] != 0x1f) 
    {
        logging.logIt(F("getUpdate"), F("Magic header does not start with 0xE9 - Update failed!"), 4);
        http.end();
        return false;
    }
    if (buf[0] == 0xe9) 
    {
        uint32_t bin_flash_size = ESP.magicFlashChipSize((buf[3] & 0xf0) >> 4);
        // check if new bin fits to SPI flash
        if(bin_flash_size > ESP.getFlashChipRealSize()) {
            logging.logIt(F("getUpdate"), F("New binary does not fit SPI Flash size - Update failed!"), 4);
            http.end();
            return false;
        }
    }
    showHeader(&http);
    if(installUpdate(*tcp, len, http.header("x-MD5"), 0)) 
    {
        logging.logIt(F("getUpdate"), F("Update Successful - Save new Version"));

        _FM->changeJsonValueFile(configFile, "softwareVersion", String(http.header("x-versionAvailiable")).c_str());
        //later implemented state set update done - later with (rebootOn Update config)
        ESP.restart();
        return true;
    }
    else 
    {
        logging.logIt(F("getUpdate"), F("Update failed! - Installation failed!"), 5);
        return false;
    }
    return false;
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
        logging.logIt(F("installUpdate"), "Error - Update.begin failed!" + String(error.c_str()), 5);
        return false;
    }
    if(md5.length()) {
        if(!Update.setMD5(md5.c_str())) {
            logging.logIt(F("installUpdate"), "Error - Update.setMD5 failed!" + String(md5.c_str()), 5);
            return false;
        }
    }
    if(Update.writeStream(in) != size) {
        Update.printError(error);
        error.trim(); // remove line ending
        logging.logIt(F("installUpdate"), "Error - Update.writeStream failed!" + String(error.c_str()), 5);
        return false;
    }
    if(!Update.end()) {
        Update.printError(error);
        error.trim(); // remove line ending
        logging.logIt(F("installUpdate"), F("Error - Update.end failed!"), 5);
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

bool OTA_Manager::getIsLastUpdateCheckFailed()
{
    return lastFailed;
}

bool OTA_Manager::getIsUpdateAvailiable()
{
    return updatesAvailiable;
}

void OTA_Manager::run()
{
    static unsigned long reInit = 0;
    static unsigned long lastCall = 0;

    if(millis() > reInit || (millis() >= 60000 && millis() <= 60500))
    {
        reInit = millis() + 3600000;
        init();
    }

    if(millis() - lastCall <= classCheckIntervall)
    {
        return;
    }

    lastCall = millis();
    _Ntp->run();
    _Network->run();
    _Wifi->run();

    //check if init is Possible
    if(!initCorrect)
    {
        if(this->init())
        {
            this->initCorrect = true;
        }
        return;
    }

    //check for updateCheck
    if(automaticUpdateSearch && initCorrect)
    {

        if(_Ntp->getEpochTime() > nextUpdateCheck)
        {
            int port = String(_FM->readJsonFileValue("config/mainConfig.json", "port")).toInt();
            bool res = this->checkForUpdates(_FM->readJsonFileValue(configFile, "updateServer"), port, _FM->readJsonFileValue(configFile, "uri"),
            _FM->readJsonFileValue(configFile, "servertoken"), _FM->readJsonFileValue(configFile, "serverpass"));

            if(!res)
            {
                logging.logIt(F("run->checkForUpdates"), F("Check for Updates return false!"));
                lastFailed = true;
            }
            else
            {
                if(lastFailed)
                {
                    lastFailed = false;
                } 
            }
        }
    }
}


/*
    Extra Stuff
    
    Only implemented for specific needs e.g fast SetupSketch for new ESP Devices
*/

bool OTA_Manager::checkForNewFilesystemUpdate(String host, uint16_t port, String uri, String username, String password, bool onlyCheck) {
    checkForFiles();
    if(!_Wifi->isConnected()) //if no network Conenction return false don't create any Objects or whatever
    {
        logging.logIt(F("checkForNewFilesystemUpdate"), F("Can't check for Updates - no Network Connection!"), 4);
        return false;
    }

    HTTPClient http; //create HTP Object to communicate with a webserver
    logging.logIt(F("checkForNewFilesystemUpdate"), "Update-Server: " + String(host) + String(" Port: ") + String(port) + String(" Uri: ") + String(uri));
    logging.logIt(F("checkForNewFilesystemUpdate"), "Username: " + String(username) + String(", password: ") + String(password), 2);

    http.begin(*_Wifi->getWiFiClient(), host, port, uri); //change with HTTPClient::begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint) - start coounication
    addHeader(&http, 4); //add basic device header

    const char * headerkeys[] = { "x-MD5", "x-requestedType", "x-requestResponse", "x-versionAvailiable"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

    // track these headers
    http.collectHeaders(headerkeys, headerkeyssize);
    http.setAuthorization(username.c_str(), password.c_str()); //read server credentials from Flash

    int code = http.GET(); 
    //int len = http.getSize();

    showHeader(&http);

    if(code <= 0)
    {
        logging.logIt(F("checkForNewFilesystemUpdate"), String("Error while checking for Filesystem updates! - Code: ") + String(code) + String("Error: ") + String(http.errorToString(code).c_str()), 4);
        return false;
    }
    else if(!http.hasHeader("x-requestedType") || !http.hasHeader("x-requestResponse"))
    {
        logging.logIt(F("checkForNewFilesystemUpdate"), String("Error while checking for FS updates! - HTTP Response is invalid! - HTTP Code: " + String(code)), 4);
        return false;
    }
    else if(http.header("x-requestedType") != String("filesystemCheck"))
    {
        logging.logIt(F("checkForNewFilesystemUpdate"), String("Error while checking for updates! - HTTP Response has an unexcepted value"), 4);
        return false;
    }

    if(http.header("x-requestedType") == String("filesystemCheck"))
    {
        if(code != 200)
        {
            logging.logIt(F("checkForNewFilesystemUpdate"), String("Error while checking for FS updates! - HTTP return unexcpeted Code: ") + String(code) + String("\n ErrorMessage: ") + String(http.errorToString(code)));
            return false;
        }
        else
        {
            if(http.header("x-requestResponse") == String("makeFSUpdate")) //device is forced to Update - only used for important update fixes like security
            {
                logging.logIt(F("checkForNewFilesystemUpdate"), "Updates found. Current FS Version: " + String(_FM->readJsonFileValue(configFile, "softwareVersion")) + String(" New Version: ") + String(http.header("x-newVersion")));
                logging.logIt(F("checkForNewFilesystemUpdate"), "Device is forced by Server to update FS! -> run Update");
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
            else if(http.header("x-requestResponse") == String("newFSVersionAvail")) //device can be updated but is not forced to - device/user settings decide if device get's the update
            {
                logging.logIt(F("checkForNewFilesystemUpdate"), "FS Updates found. Current Version: " + String(_FM->readJsonFileValue(configFile, "softwareVersion")) + String(" New Version: ") + String(http.header("x-versionAvailiable")));
                if(_FM->returnAsBool(_FM->readJsonFileValue(configFile, "autoUpdate")) && !onlyCheck) //device get auto Updates
                {
                    return getNewFilesystem(host, port, uri, username, password);
                }
                else
                {
                    return true;
                }
            }
            else if(http.header("x-requestResponse") == String("noFSUpdates"))
            {
                logging.logIt(F("checkForNewFilesystemUpdate"), F("No FS Updates"));
                return true;
            }
        }
    }
    return false;
}

bool OTA_Manager::getNewFilesystem(String host, uint16_t port, String uri, String username, String password) 
{
    _updateLed->blink(300, false, 20);
    logging.logIt(F("getNewFilesystem"), F("Start firmware Update"), 3);

/*
    if(!checkForUpdates(host, port, uri, username, password, true))
    {
        logging.logIt(F("getUpdates", "No Updates availiable! - return", 3);
        return true;
    }
    */
    HTTPClient http; //create HTP Object to communicate with a webserver
    
    http.begin(*_Wifi->getWiFiClient(), host, port, uri); //change with HTTPClient::begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint) - start coounication
    addHeader(&http, 2); //add basic device header

    const char * headerkeys[] = { "x-MD5", "x-requestedType", "x-requestResponse", "x-versionAvailiable"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    // track these headers
    http.collectHeaders(headerkeys, headerkeyssize);
    http.setAuthorization(username.c_str(), password.c_str()); //read server credentials from Flash

    int code = http.GET(); 
    int len = http.getSize();
    logging.logIt(F("getNewFilesystem"), String("HTTP Code: ") + String(code), 2);

    if(len > (int) ESP.getFreeSketchSpace()) {
        logging.logIt(F("getNewFilesystem"), "Not enough memory availiable to run the Update! Needed: " + String(len) + String(" bytes, Availiable: ") + String(ESP.getFreeSketchSpace()));
        classControl.newReport(F("Can't Update device - not enough memory!"), 2421, 2, true);
        return false;
    }

    //warn state implemented later
    WiFiClient *tcp = http.getStreamPtr();

    //WiFiUDP::stopAll(); //later implemented by cconfig
    //WiFiClient::stopAllExcept(tcp);

    int command = U_FS;

    
    showHeader(&http);
    if(installNewFilesystem(*tcp, len, http.header("x-MD5"), 0)) 
    {
        logging.logIt(F("getNewFilesystem"), F("Update Successful - Save new Version"));

        _FM->changeJsonValueFile(configFile, "softwareVersion", String(http.header("x-versionAvailiable")).c_str());
        //later implemented state set update done - later with (rebootOn Update config)
        ESP.restart();
        return true;
    }
    else 
    {
        logging.logIt(F("getNewFilesystem"), F("Update failed! - Installation failed!"), 5);
        return false;
    }
    return false;
}

bool OTA_Manager::installNewFilesystem(Stream& in, uint32_t size, const String& md5, int command)
{
    StreamString error;

    if(!Update.begin(size, U_FS)) {
        Update.printError(error);
        error.trim(); // remove line ending
        logging.logIt(F("installNewFilesystem"), "Error - Update.begin failed!" + String(error.c_str()), 5);
        return false;
    }
    if(md5.length()) {
        if(!Update.setMD5(md5.c_str())) {
            logging.logIt(F("installNewFilesystem"), "Error - Update.setMD5 failed!" + String(md5.c_str()), 5);
            return false;
        }
    }
    if(Update.writeStream(in) != size) {
        Update.printError(error);
        error.trim(); // remove line ending
        logging.logIt(F("installNewFilesystem"), "Error - Update.writeStream failed!" + String(error.c_str()), 5);
        return false;
    }
    if(!Update.end()) {
        Update.printError(error);
        error.trim(); // remove line ending
        logging.logIt(F("installNewFilesystem"), F("Error - Update.end failed!"), 5);
        return false;
    }
    return true;
}