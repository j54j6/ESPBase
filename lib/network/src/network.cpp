#include "network.h"

/*
    Constructor
*/

Network::Network(Filemanager* FM, WiFiManager* wifiManager) //basic Constructor with WifiHandler - no optical output
{
    this->FM = FM;
    this->wifiManager = wifiManager;
}

Network::Network(Filemanager* FM, WiFiManager* wifiManager, LED* wifiLed) //Default Constructor with full functionalities
{
    this->FM = FM;
    this->wifiManager = wifiManager;
    this->wifiLed = wifiLed;
}

/*
    private functions
*/

bool Network::createConfig() //Fkt Nr. 19
{
    bool res = false;
    if(!FM->fExist(configFile))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createConfig", "ConfigFile doesn't exist - try to create");
        #endif
        for(int i = 0; i < 3; i++)
        {
            FM->createFile(configFile);
            if(FM->fExist(configFile))
            {
                #ifdef J54J6_LOGGING_H
                    logging.SFLog(className, "createConfig", "File successfully created! - return");
                #endif
                break;
            }
        }
        if(!FM->fExist(configFile))
        {
            error.error = true;
            error.ErrorCode = 43;
            error.message = "Can't create Config File - 3 tries failed!";
            error.priority = 4;
        }
    }
    
    if(FM->fExist(configFile))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createConfig", "File exist - try to write in File");
        #endif
        for(int i = 0; i < 3; i++)
        {
            if(FM->writeJsonFile(configFile, configFileFallback, 12))
            {
                #ifdef J54J6_LOGGING_H
                    logging.SFLog(className, "createConfig", "Successfully created and written in File - Return");
                #endif
                res = true;
                break;
            }
        }
    }

    if(!res)
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createConfig", "Unable to create Config File - ErrorCode: 119", 2);
        #endif
        error.error = true;
        error.ErrorCode = 119;
        error.message = "Unable to create Config File";
        error.priority = 5;
        return false;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createConfig", "ConfigFile successfully created!");
        #endif
        return true;
    }
}

bool Network::createSetupFile() //Fkt. Nr 120
{
    bool res = false;
    if(!FM->fExist(setupFile))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createSetupFile", "SetupFile doesn't exist - try to create");
        #endif
        bool res = false;
        for(int i = 0; i < 3; i++)
        {
            FM->createFile(setupFile);
            if(FM->fExist(setupFile))
            {
                res = true;
                #ifdef J54J6_LOGGING_H
                    logging.SFLog(className, "createSetupFile", "File successfully created! - return");
                #endif
                break;
            }
        }
        if(!res)
        {
            #ifdef J54J6_LOGGING_H
                logging.SFLog(className, "createSetupFile", "Can't create setup File - ErrorCode: 234", 2);
            #endif
            error.error = true;
            error.ErrorCode = 234;
            error.message = "Can't create setup File";
            error.priority = 6;
            return false;
        }
    }
    
    if(FM->fExist(setupFile))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createSetupFile", "File exist - try to write in File");
        #endif
        for(int i = 0; i < 3; i++)
        {
            if(FM->writeJsonFile(setupFile, setupFileFallback, 3))
            {
                #ifdef J54J6_LOGGING_H
                    logging.SFLog(className, "createSetupFile", "Successfully created and written in File - Return");
                #endif
                res = true;
                break;
            }
        }
    }

    if(!res)
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createSetupFile", "Unable to create Setupfile - Can't write in File - ErrorCode: 120", 2);
        #endif
        error.error = true;
        error.ErrorCode = 120;
        error.message = "Unable to create Setupfile -  Can't write in File";
        error.priority = 6;
        return false;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createSetupFile", "Config File successfully created!");
        #endif
        return true;
    }
}

void Network::startWorking() //fkt. Nr. -3
{
    String staSSID = FM->readJsonFileValue(configFile, "ssid");
    String staPSK = FM->readJsonFileValue(configFile, "psk");
    

    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startSetupMode", "AP successfully started");
        String message = "Credentials: \n";
        message += " SSID: ";
        message += staSSID;
        message += "\n PSK: ";
        message += "don't hope that I log any passwords ;)";
        logging.SFLog(className, "startSetupMode", message.c_str());
    #endif

    wifiManager->startWifiStation(staSSID.c_str(), staPSK.c_str());
    runFunction = 0;
}

void Network::startSetupMode() //fkt Nr. -2
{
    Serial.println("Start setup");
    delay(500);
    String apSSID = FM->readJsonFileValue(setupFile, "ssid");
    String apPSK = FM->readJsonFileValue(setupFile, "psk");
    
    if(wifiManager->configWiFiAP(apIpAddress, apIpAddress, apNetMsk))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startSetupMode", "AP successfully configured!");
        #endif
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startSetupMode", "Can't configure AP! ERROR-Code: 23", 2);
        #endif
        error.error = true;
        error.ErrorCode = 23;
        error.message = "Can't configure AP!";
        error.priority = 5;
    }
    Serial.println("Pass1");
    delay(500);
    if(wifiManager->startWifiAP(apSSID.c_str(), apPSK.c_str()))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startSetupMode", "AP successfully started");
            String message = "Credentials: \n";
            message += " SSID: ";
            message += apSSID;
            message += "\n PSK: ";
            message += apPSK;
            logging.SFLog(className, "startSetupMode", message.c_str());
        #endif
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startSetupMode", "Can't start AP - ERROR-Code: 24", 2);
        #endif
        error.error = true;
        error.ErrorCode = 24;
        error.message = "Can't start AP!";
        error.priority = 5;
    }

    Serial.println("Pass2");
    delay(500);
    /*
        Start DNS Server
    */
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startSetupMode", "Try starting DNS Server");
    #endif
    if(startDnsServer())
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startSetupMode", "DNS Server started!");
        #endif
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startSetupMode", "Can't start DNS Server!");
        #endif
    }
    Serial.println("Pass3");
    delay(500);
    /*
        Start MDNS
    */

   if(startMDnsServer(registerHostname))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startSetupMode", "MDNS Server started!");
        #endif
    }
    else
    {
        MDNS.addService("http", "tcp", 80);
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startSetupMode", "Can't start MDNS Server!");
        #endif
    }
    Serial.println("Webserver");
    delay(500);
    /*
        Start Webserver
    */
    //addService("/", std::bind(&Network::serverHandleSetup, this));
    webserver.on("/", std::bind(&Network::serverHandleSetup, this));
    webserver.on("/cred_save", std::bind(&Network::checkAndTestCredits, this));
    webserver.onNotFound(std::bind(&Network::serverHandleCaptiveNotFound, this));
    //addNotFoundService(std::bind(&Network::serverHandleCaptiveNotFound, this));
    startWebserver(80);
    Serial.println(WiFi.localIP());
    runFunction = -1;
}

/* Private helper function
    
*/

bool Network::startDnsServer() //fkt Nr 5 / direct called in startup
{
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    if(!dnsServer.start(DNSPort, "*", apIpAddress))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startDnsServer", "Cant start DNS Server!", 2);
            
        #endif
        error.error = false;
        error.ErrorCode = 14;
        error.message = "Can't start DNS Server!";
        error.priority = 2;

        return false;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "startDnsServer", "DNS Server successfully started!");
            
        #endif
        return true;
    }
}

bool Network::startMDnsServer(const char* newHostname) //fkt Nr. 4 / direct called in startup
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startMDnsServer", "RUN - startMDnsServer");
        logging.SFLog(className, "startMDnsServer", "try starting MDNS Service");
        String message = "Hostname: ";
        message += newHostname;
        logging.SFLog(className, "startMDnsServer", message.c_str());
     #endif
    if(MDNS.begin(newHostname))
    {
        #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startMDnsServer", "MDNS Server successfully started!");
        #endif
        return true;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startMDnsServer", "ERROR Starting MDNS Server", 2);
        #endif
        error.error = false;
        error.ErrorCode = 16;
        error.message = "Can't Start MDNS Server";
        error.priority = 2;
        return false;
    }  
}

void Network::internalControl() //At this time only for performance checking
{
  this->callPerSecond = 1000/(millis() - lastCall);
}


/*
    Protected functions
*/

void Network::autoResetLock() //internal useage of delay() as sim. threading alternative
{
    if(!locked)
    {
        return;
    }
    else
    {
        if(millis() >= unlockAt)
        {
            locked = false;
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "autoResetLock", "unlock run!");
            #endif
            return;           
        }
        else
        {
            return;
        }   
    }
}

void Network::lock(ulong time) //lock run() - only handling is enabled
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        String message = "lock for ";
        message += time;
        message += "ms";
        logging.SFLog(className, "autoResetLock", message.c_str());
    #endif
    locked = true;
    unlockAt = millis() + time;
}

void Network::internalBegin()
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "internalBegin", "RUN - begin()");
        logging.SFLog(className, "internalBegin", "check for Files");
    #endif
 

    bool res = FM->fExist(configFile);

    #ifdef J54J6_LOGGING_H
        if(res)
        {
            logging.SFLog(className, "internalBegin", "configFile exist!");
        }
        else
        {
            logging.SFLog(className, "internalBegin", "configFile doesn't exist - try to create File!", 1);
        }
    #endif
    
    if(!res)
    {
        for(int i = 0; i < 3; i++)
        {
            if(createConfig())
            {   
                #ifdef J54J6_LOGGING_H
                    logging.SFLog(className, "internalBegin", "configFile successfully created", 0);
                #endif
                break;
            }
            else
            {   
                #ifdef J54J6_LOGGING_H
                    logging.SFLog(className, "internalBegin", "configFile can't be created!", 1);
                #endif
            } 
        }
        if(!FM->fExist(configFile))
        {
            #ifdef J54J6_LOGGING_H
                logging.SFLog(className, "internalBegin", "uneable to create configFile - ERRORCODE: 12", 2);
                logging.SFLog(className, "internalBegin", "disable Network! - ERRORCODE: 12", 2);
            #endif
            error.error = true;
            error.ErrorCode = 12;
            error.message = "Can't create Config File!";
            error.priority = 6;
        }
    }

    res = FM->fExist(setupFile);

    #ifdef J54J6_LOGGING_H
        if(res)
        {
            logging.SFLog(className, "internalBegin", "setupFile exist!");
        }
        else
        {
            logging.SFLog(className, "internalBegin", "setupFile doesn't exist - try to create File!", 1);
        }
    #endif
    
    if(!res)
    {
        for(int i = 0; i < 3; i++)
        {
            if(createSetupFile())
            {
                #ifdef J54J6_LOGGING_H 
                    logging.SFLog(className, "internalBegin", "setupFile successfully created", 0);
                #endif
                break;
            }
            else
            {
                #ifdef J54J6_LOGGING_H 
                    logging.SFLog(className, "internalBegin", "setupFile can't be created!", 1);
                #endif
            } 
        }
        if(!FM->fExist(setupFile))
        {
            #ifdef J54J6_LOGGING_H
                logging.SFLog(className, "internalBegin", "uneable to create setupFile - ERRORCODE: 13", 2);
                logging.SFLog(className, "internalBegin", "disable Network! - ERRORCODE: 13", 2);
            #endif
            error.error = true;
            error.ErrorCode = 13;
            error.message = "Can't create Setup File!";
            error.priority = 6;
        }
    }


    //Start Setup - all files are created
    
    if(wifiManager->setWiFiHostname(this->hostName))
    {
        #ifdef J54J6_LOGGING_H
            logging.SFLog(className, "internalBegin", "Hostname set!", 0);
        #endif
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logging.SFLog(className, "internalBegin", "can't set Hostname!", 2);
        #endif
        error.error = false;
        error.ErrorCode = 14;
        error.message = "Can't set WiFi Hostname!";
        error.priority = 2;
    }
    const char* wifiConfigured = FM->readJsonFileValue(configFile, "wiFiConfigured");
    res = FM->returnAsBool(FM->readJsonFileValue(configFile, "wiFiConfigured"));
    if(!res)
    {
        #ifdef J54J6_LOGGING_H
            logging.SFLog(className, "internalBegin", "Network not configured - start in SetupMode");
            String message = "Readed Value: ";
            message += wifiConfigured;
            message += " | ";
            message += res;
            logging.SFLog(className, "internalBegin", message.c_str());
        #endif

        //define WiFi IP Stuff
        if(wifiManager->setWiFiConfig(apIpAddress, apIpGateway, apNetMsk, dnsIP))
        {
            #ifdef J54J6_LOGGING_H
                logging.SFLog(className, "internalBegin", "WiFiConfig set!", 0);
            #endif
        }
        else
        {
            #ifdef J54J6_LOGGING_H
                logging.SFLog(className, "internalBegin", "can't set WiFiConfig!", 2);
            #endif
            error.error = false;
            error.ErrorCode = 14;
            error.message = "Can't set WiFi Config!";
            error.priority = 2;
        }



        runFunction = -2; //startSetupMode()
        return;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logging.SFLog(className, "internalBegin", "Network configured - start in WorkMode");
            String message = "Readed Value: ";
            message += wifiConfigured;
            message += " | ";
            message += res;
            logging.SFLog(className, "internalBegin", message.c_str());
        #endif
        runFunction = -3; //startWorking()
        return;
    }
}

void Network::serverHandleSetup()
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "serverHandleSetup", "setupWebHandler called!");
    #endif
    webserver.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    webserver.sendHeader("Pragma", "no-cache");
    webserver.sendHeader("Expires", "-1");
    webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    webserver.send(200, "text/html", setupPageHeader);
    webserver.sendContent(setupPageBodyPart1);
    webserver.sendContent(String(getHTMLFormattedWiFiNetworksForSetupHandler()).c_str());
    webserver.sendContent(setupPageBodyPart2);
}

void Network::checkAndTestCredits()
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "checkAndTestCredits", "credits will be checked");
    #endif
    webserver.send(102); //prevent timeout

    if(!webserver.hasArg("ssid") || webserver.arg("ssid") == "" || !webserver.hasArg("psk") || webserver.arg("psk") == "")
    {
        #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "checkAndTestCredits", "psk or ssid not set");
        #endif
        webserver.sendHeader("Location", String("http://172.20.0.1?wrongInput=true"), true);
        webserver.send( 302, "text/plain", "");
    }
    if(webserver.arg("psk").length() < 8 || webserver.arg("psk").length() > 64)
    {
        #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "checkAndTestCredits", "psk to long or short");
        #endif
        webserver.sendHeader("Location", String("http://172.20.0.1?pskfalse=true"), true);
        webserver.send( 302, "text/plain", "");
    }
    else
    {
        long startConnectTime = millis();
        uint connectTimeout = 5000;
        wifiManager->setWiFiMode(WIFI_AP_STA);
        wifiManager->startWifiStation(webserver.arg("ssid").c_str(), webserver.arg("psk").c_str(), WIFI_AP_STA);
        #ifdef J54J6_LOGGING_H
        logger logging;
        String message = "Try connect with SSID: |";
        message += webserver.arg("ssid").c_str();
        message += "| and PSk: |";
        message += webserver.arg("psk").c_str();
        message += "|";
        logging.SFLog(className, "checkAndTestCredits", message.c_str());
        #endif
        webserver.send(102);
        while(millis() <= (startConnectTime+connectTimeout))
        {
            if(wifiManager->getWiFiState() == 3)
            {
                webserver.sendHeader("Location", String("http://172.20.0.1/?success=true"), true);
                webserver.send( 302, "text/plain", "");
                #ifdef J54J6_LOGGING_H
                    logger logging;
                    logging.SFLog(className, "checkAndTestCredits", "Connection successfull");
                #endif
                break;
            }
            else
            {
                delay(50);
            }           
        }
        if(millis() > (startConnectTime+connectTimeout) || wifiManager->getWiFiState() != 3)
        {
            if(wifiManager->getWiFiState() == 1)
            {
                webserver.sendHeader("Location", String("http://172.20.0.1/?success=nossid"), true);
                webserver.send(302, "text/plain", "");
                #ifdef J54J6_LOGGING_H
                    logger logging;
                    logging.SFLog(className, "checkAndTestCredits", "Can't reach SSID anymore", 1);
                #endif
            }
            else
            {
                webserver.sendHeader("Location", String("http://172.20.0.1/?success=false"), true);
                webserver.send( 302, "text/plain", "");
                #ifdef J54J6_LOGGING_H
                    logger logging;
                    logging.SFLog(className, "checkAndTestCredits", "SSID or PSK not correct!", 1);
                #endif
            }
        }
        else
        {
            if(wifiManager->getWiFiState() == 3)
            {
                #ifdef J54J6_LOGGING_H
                    logger logging;
                    logging.SFLog(className, "checkAndTestCredits", "successfully connected to Network - save");
                #endif
                webserver.sendHeader("Location", String("http://172.20.0.1/?success=true"), true);
                webserver.send( 302, "text/plain", "");
                bool tmp = false;
                if(!saveCredentials(&webserver.arg("ssid"), &webserver.arg("psk"), configFile))
                {
                    #ifdef J54J6_LOGGING_H
                        logger logging;
                        logging.SFLog(className, "checkAndTestCredits", "Can't save WiFi credentials - saveCredentials returns false ", 2);
                    #endif
                    tmp = true;
                }
                if(!FM->changeJsonValueFile(configFile, "wiFiConfigured", "true"))
                {
                    #ifdef J54J6_LOGGING_H
                        logging.SFLog(className, "checkAndTestCredits", "Can't change to configured Network - FM::changeJsonValueFile returns false!", 2);
                    #endif
                    tmp = true;
                }
                if(!tmp)
                {
                    #ifdef J54J6_LOGGING_H
                        logging.SFLog(className, "checkAndTestCredits", "Credentials successfully saved!");
                    #endif
                }
            }
        }   
    }    
}


void Network::serverHandleCaptiveNotFound()
{
    webserver.sendHeader("Location", String("http://172.20.0.1"), true);
    webserver.send( 302, "text/plain", "");
    return;
}

/*
    Public functions
*/

        /*
            Class Control
        */
void Network::begin() //check for Files and init Network Communication - set as in configFile or setupFile
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "begin", "SET - begin()");
    #endif
    runFunction = 1;
    return;
}

void Network::startSetup() //start Fkt. Nr 2
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "begin", "SET - startSetup()");
    #endif
    runFunction = 2;
    return;
}

void Network::startWifiAP()
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "begin", "SET - startWifiAP()");
    #endif
    runFunction = 3;
    return;
}

void Network::mdnsStart()
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "begin", "SET - mDNSStart()");
    #endif
    runFunction = 4;
    return;
}

void Network::dnsStart()
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "begin", "SET - dnsStart()");
    #endif
    runFunction = 5;
    return;
}


        /*
            Get Stuff
        */
ulong Network::getCallPerSecond()
{
    return this->callPerSecond;
}

bool Network::getClassDisabled()
{
    return this->classDisabled;
}


        /*
            Set Stuff
        */
void Network::setClassDisabled(bool newVal)
{
    #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Update ClassDisabled: ";
      message += classDisabled;
      message += " -> ";
      message += newVal;
      logging.SFLog(className, "setClassDisabled", message.c_str());
  #endif
  this->classDisabled = newVal;
}

bool Network::saveCredentials(const String* ssid, const String* psk, const char* File)
{
  if(!FM->fExist(File))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "saveCredentials", "Can't save credentials - File doesn't exist!", 2);
    #endif
    return false;
  }
  else
  {
    if(!FM->changeJsonValueFile(File, "ssid", ssid->c_str()))
    {
      #ifdef J54J6_LOGGING_H
          logger logging;
          logging.SFLog(className, "saveCredentials", "Can't change Json Valie ssid! - function return false", 2);
      #endif
    }
    if(!FM->changeJsonValueFile(File, "psk", psk->c_str()))
    {
      #ifdef J54J6_LOGGING_H
          logger logging;
          logging.SFLog(className, "saveCredentials", "Can't change Json Valie psk! - function return false", 2);
      #endif
    }
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "saveCredentials", "Credentials successfully saved");
    #endif
    return true;
  }
  return false;
}

/*
    RUN - add to loop() function!
*/
void Network::run()
{
    if(millis() < (lastCall + checkDelay))
    {
        return;
    }
    internalControl();
    autoResetLock();
    dnsServer.processNextRequest();
    MDNS.update();
    webserver.handleClient();
    if(locked || classDisabled)
    {
        return;
    }
    else
    {
        switch(this->runFunction)
        {
            case -3:
                startWorking();
                break;
            case -2:
                startSetupMode();
                break;
            case 1:
                internalBegin();;
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                startDnsServer();
                break;
            default:
                break;
        }
    }
    this->lastCall = millis();
    return;
}


/*
    Webserver functionalities
*/

bool Network::startWebserver(int port)
{
    if(!this->webServerActive)
    {
        #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startWebserver", "Start Webserver");
        #endif
        webServerActive = true;
        webserver.begin(port);
        return true;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startWebserver", "Webserver already active - report", 1);
        #endif

        this->error.error = false;
        this->error.message = "Webserver already active!";
        this->error.priority = 2;
        return false;
    }    
}

bool Network::stopWebserver()
{
    if(this->webServerActive)
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "stopWebserver", "Stop Webserver");
        #endif
        webServerActive = false;
        webserver.stop();
        return true;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "stopWebserver", "Webserver already disabled", 1);
        #endif
        return false;
    }
    return false;
}

void Network::addService(const char *url, webService function)
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        String message = "Add Service: ";
        message += url;
        logging.SFLog(className, "addService", message.c_str());
    #endif
    webserver.on(url, function);
}

void Network::addNotFoundService(webService function)
{
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "addNotFoundService", "Add NotFound Site!");
    #endif
    webserver.onNotFound(function);
}

ESP8266WebServer* Network::getWebserver()
{
    return &webserver;
}


/*
    Specific Stuff
*/
        /*
            Setuphandler functions
        */
String Network::getHTMLFormattedWiFiNetworksForSetupHandler()
{
    String part;
    int amountNetworks = WiFi.scanNetworks();
    if(amountNetworks == 0)
    {
        part = "<li>Keine netzwerke gefunden!</li>";
        return part;
    }
    for(int i = 0; i < amountNetworks; i++)
    {
      part += "<li><a href='#passwordField' onclick='c(this)'>";
      part += WiFi.SSID(i);
      part += "</a>";
      part += "&nbsp;";
      part += "<span>";
      part += getRSSIasQuality(WiFi.RSSI(i));
      part += "%</span></li>";
      part += "<br/>";
    }
    return part;
}

int Network::getRSSIasQuality(int RSSI)
{
  int quality = 0;
  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}


/*
    Inherited overwritten functionalities
*/
void Network::startClass()
{
    if(this->classDisabled)
    {
        this->classDisabled = false;
        this->begin();
    }
    else
    {
        this->begin();
    }
}

void Network::stopClass()
{
    if(!this->classDisabled)
    {
        this->classDisabled = true;
        this->run();
        Serial.println("locked!");
        return;
    }
}

void Network::pauseClass()
{
    this->stopClass();
}

void Network::restartClass()
{
    this->startClass();
}

void Network::continueClass()
{
    this->startClass();
}