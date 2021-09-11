#include "network.h"


Network::Network(Filemanager* FM, WiFiManager* wifiManager)
{
    this->_FM = FM;
    this->wifiManager = wifiManager;
}

Network::Network(Filemanager* FM, WiFiManager* wifiManager, LED* wifiLed)
{
    this->_FM = FM;
    this->wifiManager = wifiManager;
    this->wifiLed = wifiLed;
}


bool Network::createConfig()
{
    const char* configFileFallback[12][2] = {
            {"ssid", ""},
            {"psk", ""},
            {"wiFiConfigured", "false"},
            {"state", "notConfigured"},
            {"wifiMode", "STA"},
            {"wifiAutoTime", "3600"},
            {"hostType", "hidden"},
            {"channel", ""},
            {"bssid", ""},
            {"hostname", ""},
            {"apSSID", ""},
            {"apPSK", ""}
        };
    bool res = false;
    _FM->begin();
    _FM->fDelete(configFile);
    if(!_FM->fExist(configFile))
    {
        #ifdef J54J6_SysLogger
            logging.logIt(F("createConfig"), F("ConfigFile doesn't exist - try to create"));
        #endif
        for(int i = 0; i < 3; i++)
        {
            _FM->createFile(configFile);
            if(_FM->fExist(configFile))
            {
                #ifdef J54J6_SysLogger
                    logging.logIt(F("createConfig"), F("File successfully created! - return"));
                #endif
                break;
            }
        }
        if(!_FM->fExist(configFile))
        {
            classControl.newReport(F("Can't create Config File - 3 tries failed!"), 43, 4, true);
        }
    }
    
    if(_FM->fExist(configFile))
    {
        #ifdef J54J6_SysLogger
            
            logging.logIt(F("createConfig"), F("File exist - try to write in File"));
        #endif
        for(int i = 0; i < 3; i++)
        {
            if(_FM->writeJsonFile(configFile, configFileFallback, 12))
            {
                #ifdef J54J6_SysLogger
                    logging.logIt(F("createConfig"), F("Successfully created and written in File - Return"));
                #endif
                res = true;
                break;
            }
        }
    }

    if(!res)
    {
        #ifdef J54J6_SysLogger
            
            logging.logIt(F("createConfig"), F("Unable to create Config File - ErrorCode: 119"), 2);
        #endif
        classControl.newReport("Unable to create Config File", 119, 5, true);
        return false;
    }
    else
    {
        #ifdef J54J6_SysLogger
            
            logging.logIt(F("createConfig"), F("ConfigFile successfully created!"));
        #endif
        return true;
    }
}


bool Network::initSetup()
{
    //init wifi AP
    logging.logIt(F("initSetup"), F("Init. WiFi AP"));
        
    if(!this->wifiManager->setWiFiMode(WIFI_AP_STA))
    {
        logging.logIt(F("initSetup"), F("Can't set WiFi Mode to STA_AP!"), 6);
        classControl.newReport("Error while configuring WiFi AP to STA_AP", 542, 5, true);
    }

    if(!this->wifiManager->setSoftAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)))
    {
        logging.logIt(F("initSetup"), F("Can't set WiFi Config - ERROR!"), 6);
        classControl.newReport("Error while configuring WiFi AP", 542, 5, true);
        return false;
    }
    logging.logIt(F("initSetup"), F("WiFi Module configured"), 4);

    if(!this->wifiManager->configWiFiAP(apIP, apIP, IPAddress(255,255,255,0)))
    {
        logging.logIt(F("initSetup"), F("Can't set WiFi AP Config - ERROR!"), 6);
        classControl.newReport("Error while configuring WiFi AP Config", 542, 5, true);
        return false;
    }
    logging.logIt(F("initSetup"), F("WiFi AP Configured"), 4);

    if(!this->wifiManager->startWifiAP(backupAPSSID, backupAPPSK))
    {
        logging.logIt(F("initSetup"), F("Can't start WiFi AP! - ERROR!"), 6);
        classControl.newReport("Error while starting WiFi AP Config", 542, 5, true);
        return false;
    }
    else
    {
        logging.logIt(F("initSetup"), F("WiFI AP started!"), 3);
    }
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    if(!this->startDNSServer(DNS_PORT, "*", apIP))
    {
        logging.logIt(F("initSetup"), F("Can't start DNS Server!"), 4);
    }
    else
    {
        logging.logIt(F("initSetup"), F("DNS Started!"), 3);
    }

    if(!MDNS.addService("http", "tcp", 80))
    {
        logging.logIt(F("initSetup"), F("Can't add MDNS HTTP Service!"), 4);
    }
    else
    {
        logging.logIt(F("initSetup"), F("mDNS Service added!"), 3);
    }

    if(!MDNS.begin(hostName))
    {
        logging.logIt(F("initSetup"), F("Can't start mDNS Server!"), 4);
    }
    else
    {
        logging.logIt(F("initSetup"), F("mDNS Started!"), 3);
    }

    webserver.on("/", std::bind(&Network::sendWiFiSite, this));
    webserver.on("/cred_save", std::bind(&Network::checkAndTestCredits, this));
    webserver.onNotFound(std::bind(&Network::sendWiFiSite, this));

/*
    webserver.on("/spec-exp.css", std::bind(&Network::sendspecexpcss, this));
    webserver.on("/spec.css", std::bind(&Network::sendSpecCss, this));
    webserver.on("/spec-icons.css", std::bind(&Network::sendSpecIcons, this));
    webserver.on("/own.css", std::bind(&Network::sendownCSS, this));
*/

    if(!this->wifiManager->setWiFiHostname("NodeworkNewDevice"))
    {
        logging.logIt(F("initSetup"), F("Can't set WiFi Hostname!"), 4);
    }
}

/*
    Website Stuff
*/

bool Network::saveCredentials(const String* ssid, const String* psk, const char* File)
{
  if(!_FM->fExist(File))
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt(F("saveCredentials"), F("Can't save credentials - File doesn't exist!"), 2);
    #endif
    return false;
  }
  else
  {
    if(!_FM->changeJsonValueFile(File, "ssid", ssid->c_str()))
    {
      #ifdef J54J6_SysLogger
          
          logging.logIt(F("saveCredentials"), F("Can't change Json Valie ssid! - function return false"), 2);
      #endif
    }
    if(!_FM->changeJsonValueFile(File, "psk", psk->c_str()))
    {
      #ifdef J54J6_SysLogger
          
          logging.logIt(F("saveCredentials"), F("Can't change Json Valie psk! - function return false"), 2);
      #endif
    }
    #ifdef J54J6_SysLogger
        
        logging.logIt(F("saveCredentials"), F("Credentials successfully saved"));
    #endif
    return true;
  }
  return false;
}

void Network::checkAndTestCredits()
{
    if(!webserver.hasArg("ssid") || webserver.arg("ssid") == "" || !webserver.hasArg("psk") || webserver.arg("psk") == "")
    {
        logging.logIt(F("checkAndTestCredits"), F("psk or ssid not set"));
        webserver.sendHeader("Location", String("http://192.168.178.1?wrongInput=true"), true);
        webserver.send( 302, "text/plain", "");
    }
    if(webserver.arg("psk").length() < 8 || webserver.arg("psk").length() > 64)
    {
        logging.logIt(F("checkAndTestCredits"), F("psk to long or short"));
        webserver.sendHeader(F("Location"), String("http://192.168.178.1?pskfalse=true"), true);
        webserver.send( 302, F("text/plain"), "");
    }
    else
    {
        logging.logIt(F("checkAndTestCredits"), F("Check SSID and PSK"));

        String ssid = webserver.arg("ssid");
        String psk = webserver.arg("psk");
        if(!wifiManager->startWifiStation(ssid.c_str(), psk.c_str(), WIFI_AP_STA, false, 2))
        {
            logging.logIt(F("checkAndTestCredits"), F("Can't start WiFi Station!"), 5);
            classControl.newReport("Can't start WiFi Station", 942, 5, true);
            ESP.reset();
        }
        logging.logIt(F("checkAndTestCredits"), F("WiFI Station started"));
        ulong start = millis();

        while(millis() < start + 5000)
        {
            yield();
        }
        if(!WiFi.isConnected())
        {
            logging.logIt(F("checkAndTestCredits"), F("SSID or PSK may be wrong"));
            webserver.sendHeader("Location", String("http://192.168.178.1/?success=false"), true);
            webserver.send( 302, "text/plain", "");
        }
        else
        {
            logging.logIt(F("checkAndTestCredits"), F("SSID or PSK are correct"));
            if(saveCredentials(&webserver.arg("ssid"), &webserver.arg("psk"), configFile))
            {
                logging.logIt(F("checkAndTestCredits"), F("SSID and PSK are saved"));
                webserver.sendHeader("Location", String("http://192.168.178.1/?success=true"), true);
                webserver.send( 302, "text/plain", "");
                if(_FM->changeJsonValueFile(configFile, "wiFiConfigured", "true"))
                {
                    logging.logIt(F("checkAndTestCredits"), F("Network successfully configured"));
                    this->begin();
                }
                else
                {
                    logging.logIt(F("checkAndTestCredits"), F("Error while set Network to 'configured'!"), 5);
                    classControl.newReport(F("Error while set Network to 'configured'!"), 834, 5, true);
                    return;
                }
            }
            else
            {
                logging.logIt(F("checkAndTestCredits"), F("Error while saving credentails"));
                webserver.sendHeader("Location", String("http://192.168.178.1/?success=false"), true);
                webserver.send( 302, "text/plain", "");
            }
        }
        webserver.handleClient();
    }
}


void Network::sendspecexpcss()
{
  //Serial.println("spec exp called!");
  ESP8266WebServer* webserver = this->getWebserver();

    /*
  File specExp = _FM->fdOpen("config/os/web/spec-exp.css", "r");
  if (webserver->streamFile(specExp, "text/css") != specExp.size()) 
  {
    logging.logIt(F("sendspecexpcss"), F("Error while Streaming CSS File!"), 4);
  }
    
  specExp.close();
  */
    webserver->send(200, "text/css", specExpCssRaw1);
    webserver->sendContent(specExpCssRaw2);
    webserver->sendContent(specExpCssRaw3);
  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec-exp.css"));
}

void Network::sendownCSS()
{
  ESP8266WebServer* webserver = this->getWebserver();

    /*
  File specExp = _FM->fdOpen("config/os/web/own.css", "r");
  if (webserver->streamFile(specExp, "text/css") != specExp.size()) 
  {
    logging.logIt(F("sendownCSS"), F("Error while Streaming CSS File!"), 4);
  }
 
  specExp.close();
  */
    webserver->send(200, "text/css", OwnCssRaw);
  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec-exp.css"));
}

void Network::sendSpecCss()
{   
  //Serial.println("spec csscalled!");
  ESP8266WebServer* webserver = this->getWebserver();

    /*
  File specFile = _FM->fdOpen("config/os/web/spec.css", "r");
  if (webserver->streamFile(specFile, "text/css") != specFile.size()) 
  {
    logging.logIt(F("sendSpecCss"), F("Error while Streaming CSS File!"), 4);
  }
 
  specFile.close();
  */
    webserver->send(200, "text/css", specCssRaw1);
    webserver->sendContent(specCssRaw2);
    webserver->sendContent(specCssRaw3);
    webserver->sendContent(specCssRaw4);
    webserver->sendContent(specCssRaw5);
    webserver->sendContent(specCssRaw6);
  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec.css"));
}

void Network::sendSpecIcons()
{
  //Serial.println("send Spec Icons called!");
  ESP8266WebServer* webserver = this->getWebserver();
  /*
  File specIcons = _FM->fdOpen("config/os/web/spec-icons.css", "r");
  if (webserver->streamFile(specIcons, "text/css") != specIcons.size()) 
  {
    logging.logIt(F("sendSpecIcons"), F("Error while Streaming CSS File!"), 4);
  }
  specIcons.close();
  */

 webserver->send(200, "text/css", specIconsCssRaw);
}

String Network::getHTMLFormattedWiFiNetworksForSetupHandler()
{
    String part;
    int amountNetworks = WiFi.scanNetworks();
    if(amountNetworks == 0)
    {
        part = F("<li>Keine netzwerke gefunden!</li>");
        return part;
    }
    for(int i = 0; i < amountNetworks; i++)
    {
      part += F("<li><a href='#passwordField' onclick='c(this)'>");
      part += WiFi.SSID(i);
      part += F("</a>");
      part += F("&nbsp;");
      part += F("<span>");
      part += getRSSIasQuality(WiFi.RSSI(i));
      part += F("%</span></li>");
      part += F("<br/>");
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


void Network::sendWiFiSite()
{
    webserver.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    webserver.sendHeader(F("Pragma"), F("no-cache"));
    webserver.sendHeader(F("Expires"), F("-1")); 
    webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    /*
     //webserver->send(200, "image/png", FM->readFile("config/os/web/logo.PNG"));
    String websitePrepared = setupPageContent;
    String templateSite = setupPageTemplate1;
    templateSite += setupPageTemplate2;


    templateSite.replace("%content%", websitePrepared);
    templateSite.replace("%wifiActive%", "active-subGroup");
    templateSite.replace("%ssid%", WiFi.SSID());
    */
    webserver.send(200, "text/html", setupPageHeader);
    webserver.sendContent(setupPageBodyPart1);
    webserver.sendContent(String(getHTMLFormattedWiFiNetworksForSetupHandler()).c_str());
    webserver.sendContent(setupPageBodyPart2);

}

/*
    Modules
*/

bool Network::startDNSServer(uint16 port, String domainName, IPAddress resolveIP)
{
    return dnsServer.start(port, domainName, resolveIP);
}


/*
    Webserver
*/

void Network::startWebserver(int port)
{
    logging.logIt(F("startWebserver"), F("Webserver started"), 3);
    webserverActive = true;
    this->webserver.begin(port);
}

void Network::stopWebserver()
{
    logging.logIt(F("stopWebserver"), F("Stopped Webserver"), 3);
    webserverActive = false;
    this->webserver.stop();
}

void Network::addService(const char *url, webService function)
{
    bool restart = false;
    if(webserverActive)
    {
        restart = true;
        stopWebserver();
    }
    webserver.on(url, function);
    logging.logIt(F("addService"), "Webservice " + String(url) + " added!", 3);
    if(restart)
    {
        startWebserver(80);
    }
}

void Network::addNotFoundService(webService function)
{
    logging.logIt(F("addNotFoundService"), F("Add NotFound Site!"));
    bool restart = false;
    if(webserverActive)
    {
        restart = true;
        stopWebserver();
    }
    webserver.onNotFound(function);
    if(restart)
    {
        startWebserver(80);
    }
}

ESP8266WebServer* Network::getWebserver()
{
    return &webserver;
}


bool Network::getDeviceIsConfigured()
{
    return this->workMode;
}

bool Network::updateHostname(const char* newHostname)
{
    String toSetHostname;
    if(wifiManager->isConnected())
    {
        if(strcmp(newHostname, "") == 0)
        {
            logging.logIt(F("updateHostname"), F("no Hostname given! - use MAC"), 3);
            toSetHostname = wifiManager->getStationMacAsString();
        }
        toSetHostname.replace(":", "-");
        if(wifiManager->setWiFiHostname(toSetHostname.c_str()))
        {
            #ifdef J54J6_SysLogger
                logging.logIt(F("updateHostname"), "Hostname '" + toSetHostname + "' set!", 3);
            #endif
            return true;
        }
        else
        {
            #ifdef J54J6_SysLogger
                logging.logIt(F("updateHostname"), "can't set Hostname: " + String(toSetHostname), 2);
            #endif
            classControl.newReport("Can't set WiFi Hostname: " + String(toSetHostname), 14, 2);
        }
    }
    return false;   
}


void Network::startWorking()
{
    if(_FM->fExist(configFile))
    {
        logging.logIt(F("startWorking"), F("Network started - read Config"));
        String ssid = _FM->readJsonFileValue(configFile, "ssid");
        String psk = _FM->readJsonFileValue(configFile, "psk");
        String hostname = _FM->readJsonFileValue(configFile, "hostname");
        String pwifiMode = _FM->readJsonFileValue(configFile, "wifiMode");
        WiFiMode_t wifiModuleMode = WIFI_STA;

        if(hostname.length() < 2)
        {
            logging.logIt(F("startWorking"), F("No custom Hostname set - use Mac!"));
            hostname = WiFi.macAddress();
        }
        logging.logIt(F("startWorking"), "WifiMode: " + pwifiMode);
        if(pwifiMode == "STA" || pwifiMode == "AP_STA") //start STA
        {
            if(pwifiMode == "STA")
            {
                logging.logIt(F("startWorking"), F("WiFi Mode => STA -> connect to AccessPoint"));
                wifiModuleMode = WIFI_STA;
            }
            else
            {
                logging.logIt(F("startWorking"), F("WiFi Mode => AP_STA -> start AP"));
                wifiModuleMode = WIFI_AP_STA;
            }

            
            if(wifiManager->startWifiStation(ssid.c_str(), psk.c_str(), wifiModuleMode, false))
            {
                logging.logIt(F("startWorking"), F("Successfully started AccessPoint connect"));

            }
        }

        if(pwifiMode == "AP" || pwifiMode == "AP_STA") //start AP
        {
            
            if(pwifiMode == "AP_STA")
            {
                logging.logIt(F("startWorking"), F("WiFi Mode => AP_STA -> start AP"));
                wifiModuleMode = WIFI_STA;
            }
            else
            {
                logging.logIt(F("startWorking"), F("WiFi Mode => AP -> start AP"));
                wifiModuleMode = WIFI_AP_STA;
            }

            wifiManager->configWiFiAP(IPAddress(10,0,0,1), IPAddress(10,0,0,1), IPAddress(255,255,255,0));
            String apSSID = _FM->readJsonFileValue(configFile, "apSSID");
            String apPSK = _FM->readJsonFileValue(configFile, "apPSK");
            bool hiddenHost = _FM->returnAsBool(_FM->readJsonFileValue(configFile, "apHide"));

            if(apSSID.length() < 5)
            {
                logging.logIt(F("startWorking"), F("No APSSID defined - use saved client ssid and password"));
                apSSID = ssid;
                apPSK = psk;
            }
            
            bool apRes = wifiManager->startWifiAP(apSSID.c_str(), apPSK.c_str(), hiddenHost);
            if(apRes)
            {
                logging.logIt(F("startWorking"), F("AP Successfully started!"));
            }
            else
            {
                logging.logIt(F("startWorking"), F("Can't start WiFI AP!"));
            }
        }
    }
    else
    {
        logging.logIt(F("startWorking"), F("no Config found!"));
        delay(1000);
        ESP.restart();
    }

    
}

void Network::begin()
{
    logging.logIt(F("begin"), F("Init. Network"));
    _FM->begin();
    if(!_FM->fExist(configFile))
    {
        logging.logIt(F("begin"), F("No Config File! - create File"));
        if(createConfig())
        {
            begin();
        }
        else
        {
            ESP.restart();
        }

    }
    else
    {
        logging.logIt(F("begin"), F("Network configured - start workmode"));
        if(_FM->returnAsBool(_FM->readJsonFileValue(configFile, "wiFiConfigured")))
        {
            workMode = true; //start working
            startWorking();
        }
        else
        {
            initSetup();
        }
    }
}

void Network::run()
{
    wifiManager->run();
    dnsServer.processNextRequest();
    MDNS.update();
    webserver.handleClient();

    if(!hostnameAlreadySet & workMode)
    {
        String hostname = _FM->readJsonFileValue(configFile, "hostname");
        hostnameAlreadySet = updateHostname(hostname.c_str());
    }
}