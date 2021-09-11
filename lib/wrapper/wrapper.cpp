#include "wrapper.h"

Filemanager* espOS::getFilemanagerObj()
{
    return this->_FM;
}

WiFiManager* espOS::getWiFimanagerObj()
{
    return this->_Wifi;
}

Network* espOS::getNetworkManagerObj()
{
    return this->_Network;
}

NTPManager* espOS::getNTPManagerObj()
{
    return this->_Ntp;
}

MQTTHandler* espOS::getMqttHandler()
{
    return this->_mqttHandler;
}

OTA_Manager* espOS::getOtaManagerObject()
{
    return this->_OTA;
}

voltageDetector* espOS::getVoltageDetectorObj()
{
    return this->_voltageDetector;
}

void espOS::addModuleToWatchdog(ClassModuleSlave* newModule)
{
    this->_Watcher->addModuleSlave(newModule);
}

void espOS::disableLeds()
{
    _WorkLed->disable();
}

void espOS::showStorage() {
          uint storage = ESP.getFlashChipRealSize();
          uint freeSpace = ESP.getFreeHeap();
          
          freeSpace = freeSpace / 1000;
          _initLogger->logIt(F("handleMainStateSite"), "Free Heap: " + String(freeSpace));
          _initLogger->logIt(F("handleMainStateSite"), "ESP total: " + String(storage));
          _initLogger->logIt(F("handleMainStateSite"), F("----"));
}

void espOS::handleInfoSite()
{
  ESP8266WebServer* webserver = _Network->getWebserver();
    webserver->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    webserver->sendHeader(F("Pragma"), F("no-cache"));
    webserver->sendHeader(F("Expires"), F("-1"));
    webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    //webserver->send(200, "image/png", FM->readFile("config/os/web/logo.PNG"));
    String websitePrepared = _FM->readFile("config/os/web/info.html");
    String templateSite = _FM->readFile("config/os/web/template.html");



    templateSite.replace("%content%", websitePrepared);
    templateSite.replace("%infoActive%", "active-subGroup");

    templateSite.replace("%firmwareVersion%", String(_FM->readJsonFileValue("config/mainConfig.json", "softwareVersion")));
    templateSite.replace("%hostname%", String(WiFi.hostname()));
     templateSite.replace("%errorCounter%", String(_initLogger->getErrorCounter()));
    templateSite.replace("%uptime%", String(millis()/1000/60) + String(" Minuten"));
    templateSite.replace("%wifiSSID%", String(WiFi.SSID()));
    templateSite.replace("%ipAdress%", String(WiFi.localIP().toString()));
    templateSite.replace("%macAddress%", String(WiFi.macAddress()));
    templateSite.replace("%subnetMask%", String(WiFi.subnetMask().toString()));
    templateSite.replace("%channel%", String(WiFi.channel()));
    templateSite.replace("%dnsAdress%", String(WiFi.dnsIP().toString()));

    webserver->send(200, "text/html", templateSite);

}

void espOS::handleLogSite()
{
  ESP8266WebServer* webserver = _Network->getWebserver();
  
    webserver->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    webserver->sendHeader(F("Pragma"), F("no-cache"));
    webserver->sendHeader(F("Expires"), F("-1"));
    webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    //webserver->send(200, "image/png", FM->readFile("config/os/web/logo.PNG"));
    String websitePrepared = _FM->readFile("config/os/web/log.html");
    String templateSite = _FM->readFile("config/os/web/template.html");


    templateSite.replace("%content%", websitePrepared);
    templateSite.replace("%logsActive%", "active-subGroup");
    

    webserver->send(200, "text/html", templateSite);
}

//Website Stuff
void espOS::handleMainStateSite()
{
    ESP8266WebServer* webserver = _Network->getWebserver();
    webserver->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    webserver->sendHeader(F("Pragma"), F("no-cache"));
    webserver->sendHeader(F("Expires"), F("-1"));
    webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    //webserver->send(200, "image/png", FM->readFile("config/os/web/logo.PNG"));
    String websitePrepared = _FM->readFile("config/os/web/status.html");
    String templateSite = _FM->readFile("config/os/web/template.html");


    templateSite.replace("%content%", websitePrepared);
    templateSite.replace("%statusActive%", "active-subGroup");
    String connected;
    if(_Wifi->isConnected() == 1)
    {
      templateSite.replace("%WiFiStatusClass%", "textOkay");
      connected = "Connected";
    }
    else{
      templateSite.replace("%WiFiStatusClass%", "textError");
      connected = "Not Connected!";
    }
    templateSite.replace("%WiFiStatus%", connected);

    if(_mqttHandler->isConnected())
    {
      templateSite.replace("%MQTTStatusClass%", "textOkay");
      templateSite.replace("%MQTTStatus%", "Connected");
    }
    else
    {
      templateSite.replace("%MQTTStatusClass%", "textError");
      templateSite.replace("%MQTTStatus%", "Not Connected");
    }
    
    if(_OTA->getIsLastUpdateCheckFailed())
    {
      templateSite.replace("%UpdaterStatusClass%", "textError");
      templateSite.replace("%updaterStatus%", "FAILED!");
    }
    else
    {
      if(_OTA->getIsUpdateAvailiable())
      {
          templateSite.replace("%UpdaterStatusClass%", "textWarn");
          templateSite.replace("%updaterStatus%", "Update available");
      }
      else
      { 
        templateSite.replace("%UpdaterStatusClass%", "textOkay");
        templateSite.replace("%updaterStatus%", "Okay");
      }
      
    }

    uint32_t RamUsage = (81920 - ESP.getFreeHeap())/1000;
    float RamInPercent = 81920 - ESP.getFreeHeap();
    RamInPercent = RamInPercent/81920;
    RamInPercent = RamInPercent * 100;
    templateSite.replace("%BatteryUseageVolt%", String(_voltageDetector->getActualVoltage()));
    templateSite.replace("%BatteryUseagePercent%", String(_voltageDetector->getPercent()));
    templateSite.replace("%RamUseageKB%", String(RamUsage));
    templateSite.replace("%RamUseagePercent%", String(RamInPercent));
    
    uint storage = ESP.getFlashChipRealSize();
    uint freeSpace = storage - ESP.getFreeSketchSpace();
    freeSpace = freeSpace / 1000;
    _initLogger->logIt(F("handleMainStateSite"), "Free Space: " + String(freeSpace));
    _initLogger->logIt(F("handleMainStateSite"), "ESP total: " + String(storage));
    float FreeSpacePercent = freeSpace * 1000;
    FreeSpacePercent = FreeSpacePercent / storage;
    FreeSpacePercent = FreeSpacePercent * 100;
    
    templateSite.replace("%flashUseageMB%", String(freeSpace));
    templateSite.replace("%flashUseagePercent%", String(FreeSpacePercent));

    webserver->send(200, "text/html", templateSite);
}


void espOS::handleControlSite()
{
  ESP8266WebServer* webserver = _Network->getWebserver();
    webserver->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    webserver->sendHeader(F("Pragma"), F("no-cache"));
    webserver->sendHeader(F("Expires"), F("-1"));
    webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    //webserver->send(200, "image/png", FM->readFile("config/os/web/logo.PNG"));
    String websitePrepared = _FM->readFile("config/os/web/control.html");
    String templateSite = _FM->readFile("config/os/web/template.html");


    templateSite.replace("%content%", websitePrepared);
    templateSite.replace("%controlActive%", "active-subGroup");

    webserver->send(200, "text/html", templateSite);
}

void espOS::handleWifiSite()
{
  ESP8266WebServer* webserver = _Network->getWebserver();
    webserver->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    webserver->sendHeader(F("Pragma"), F("no-cache"));
    webserver->sendHeader(F("Expires"), F("-1"));
    webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    //webserver->send(200, "image/png", FM->readFile("config/os/web/logo.PNG"));
    String websitePrepared = _FM->readFile("config/os/web/wifiSettings.html");
    String templateSite = _FM->readFile("config/os/web/template.html");


    templateSite.replace("%content%", websitePrepared);
    templateSite.replace("%wifiActive%", "active-subGroup");
    templateSite.replace("%ssid%", WiFi.SSID());

    webserver->send(200, "text/html", templateSite);
}

void espOS::handleUpdateSite()
{
  ESP8266WebServer* webserver = _Network->getWebserver();
    webserver->sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    webserver->sendHeader(F("Pragma"), F("no-cache"));
    webserver->sendHeader(F("Expires"), F("-1"));
    webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    //webserver->send(200, "image/png", FM->readFile("config/os/web/logo.PNG"));
    String websitePrepared = _FM->readFile("config/os/web/updateSettings.html");
    String templateSite = _FM->readFile("config/os/web/template.html");

    websitePrepared.replace("   ", "");
    templateSite.replace("   ", "");
    
    templateSite.replace("%content%", websitePrepared);
    templateSite.replace("%updateActive%", "active-subGroup");

    webserver->send(200, "text/html", templateSite);
}


/*
    Website Graphic Stuff

*/

void espOS::sendWebsiteLogo()
{
  //Serial.println("send Logo called!");
  ESP8266WebServer* webserver = _Network->getWebserver();
  File logo = _FM->fdOpen("config/os/web/logo.PNG", "r");
  if (webserver->streamFile(logo, "image/png") != logo.size()) 
  {
    _initLogger->logIt(F("sendWebsiteLogo"), F("Error while Streaming CSS File!"), 4);
  }
  logo.close();
}

void espOS::sendSpecIcons()
{
  //Serial.println("send Spec Icons called!");
  ESP8266WebServer* webserver = _Network->getWebserver();
  File specIcons = _FM->fdOpen("config/os/web/spec-icons.css", "r");
  if (webserver->streamFile(specIcons, "text/css") != specIcons.size()) 
  {
    _initLogger->logIt(F("sendSpecIcons"), F("Error while Streaming CSS File!"), 4);
  }
  specIcons.close();
}

//Website CSS Stuff
void espOS::sendspecexpcss()
{
  //Serial.println("spec exp called!");
  ESP8266WebServer* webserver = _Network->getWebserver();

  File specExp = _FM->fdOpen("config/os/web/spec-exp.css", "r");
  if (webserver->streamFile(specExp, "text/css") != specExp.size()) 
  {
    _initLogger->logIt(F("sendspecexpcss"), F("Error while Streaming CSS File!"), 4);
  }
 
  specExp.close();
  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec-exp.css"));
}

//Website CSS Stuff
void espOS::sendownCSS()
{
  //Serial.println("spec exp called!");
  ESP8266WebServer* webserver = _Network->getWebserver();

  File specExp = _FM->fdOpen("config/os/web/own.css", "r");
  if (webserver->streamFile(specExp, "text/css") != specExp.size()) 
  {
    _initLogger->logIt(F("sendownCSS"), F("Error while Streaming CSS File!"), 4);
  }
 
  specExp.close();
  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec-exp.css"));
}

void espOS::sendSpecCss()
{   
  //Serial.println("spec csscalled!");
  ESP8266WebServer* webserver = _Network->getWebserver();

  File specFile = _FM->fdOpen("config/os/web/spec.css", "r");
  if (webserver->streamFile(specFile, "text/css") != specFile.size()) 
  {
    _initLogger->logIt(F("sendSpecCss"), F("Error while Streaming CSS File!"), 4);
  }
 
  specFile.close();
  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec.css"));
}

void espOS::mqttOSCommands()
{
    static int step = 0;
    //static int lastCall = 0;
  if(_mqttHandler->getCallback()->payload != "")
  {
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "restart")
    {
      ESP.restart();
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "shutdown")
    {
      ESP.deepSleep(5000000);
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "formatDevice")
    {
      _FM->format();
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "getMac")
    {
      _mqttHandler->publish("/home/public", _Wifi->getDeviceMac().c_str());
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "update")
    {
      _OTA->getUpdatesAutoCred();
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "updateCheck")
    {
      _OTA->checkUpdatesAutoCred();
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "disableLed")
    {
      _WorkLed->disable();
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "enableLed")
    {
      _WorkLed->enable();
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "disableAutoUpdate")
    {
      _FM->changeJsonValueFile("config/mainConfig.json", "autoUpdate", "false");
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "enableAutoUpdate")
    {
      _FM->changeJsonValueFile("config/mainConfig.json", "autoUpdate", "true");
    }
    if(_mqttHandler->getCallback()->topic == "home/control" && _mqttHandler->getCallback()->payload == "enableConfMode")
    {
      _initLogger->logIt(F("mqttOSCommands"), F("MQTT ConfMode enabled"));
      _mqttConfigurator->setConfMode(true);
    }
  }


  if(_Network->getDeviceIsConfigured())
  {
    if(_Wifi->isConnected())
    {
      if(step > 1 && !_mqttHandler->isConnected())
      {
        _initLogger->logIt(F("mqttOSCommands"), F("MQTT Broker - Connection Lost - reconnect!"), 4);
        step = 0;
      }

      switch(step) {
        case 0:
          
          if(_mqttHandler->connect())
          {
            //Serial.println("Successfully loaded Connect Config!");
            step++;
            break;
          }
          /*
          else
          {
            Serial.println("Error connectng with MQTT Broker!");
          }
          */
          break;
        
        case 1:
          if(_mqttHandler->isConnected())
          {
            _initLogger->logIt(F("mqttOSCommands"), F("Connected with Broker!"));
            step++;
            //lastCall = millis();
            _mqttHandler->subscribe("home/test");
            _mqttHandler->subscribe("home/control");
          }
          break;

        case 2:
          if(_mqttHandler->publish("home/test", "ESP works!"))
          {
            _initLogger->logIt(F("mqttOSCommands"), F("Startmessage successfully published!"));
            step++;
          }
          else
          {
            _initLogger->logIt(F("mqttOSCommands"), F("Error while publishing MQTT Start Message"), 4);
          }
          break;
      }
    }
  }
}



void espOS::begin()
{
    Serial.begin(921600);
    _FM->mount();
    _initLogger->setMqttClient(_mqttHandler->getPubSubClient());
    _FM->getSerialFileStructure();
    _Ntp->begin();
    _serviceHandler->beginListen();
    _Network->begin();
    _mqttHandler->init();
    _mqttConfigurator->begin();
    _Watcher->addModuleSlave(_Wifi->getClassModuleSlave());
    _Watcher->addModuleSlave(_Network->getClassModuleSlave());
    _Watcher->addModuleSlave(_mqttHandler->getClassModuleSlave());
    _Watcher->addModuleSlave(_serviceHandler->getClassModuleSlave());
    _Watcher->addModuleSlave(_mqttConfigurator->getClassModuleSlave());


    //Add State Main Website
    _Network->stopWebserver();
    _Network->addService("/", std::bind(&espOS::handleInfoSite, this));
    _Network->addService("/status", std::bind(&espOS::handleMainStateSite, this));
    _Network->addService("/info", std::bind(&espOS::handleInfoSite, this));
    _Network->addService("/log", std::bind(&espOS::handleLogSite, this));
    _Network->addService("/control", std::bind(&espOS::handleControlSite, this));
    _Network->addService("/wifisettings", std::bind(&espOS::handleWifiSite, this));
    _Network->addService("/updatesettings", std::bind(&espOS::handleUpdateSite, this));
    _Network->addService("/spec-exp.css", std::bind(&espOS::sendspecexpcss, this));
    _Network->addService("/spec.css", std::bind(&espOS::sendSpecCss, this));
    _Network->addService("/spec-icons.css", std::bind(&espOS::sendSpecIcons, this));
    _Network->addService("/own.css", std::bind(&espOS::sendownCSS, this));
    _Network->addService("/logo.png", std::bind(&espOS::sendWebsiteLogo, this));
    _Network->addService("/logo", std::bind(&espOS::sendWebsiteLogo, this));
    _Network->startWebserver(80);
}

void espOS::run() {
    this->_WifiLed->run();
    this->_WorkLed->run();
    this->_errorLed->run();
    this->_Network->run();
    this->_Wifi->run();
    this->_serviceHandler->loop();
    this->_Watcher->run();
    this->_mqttHandler->run();
    this->_voltageDetector->run();
    this->_mqttConfigurator->run();

    if(_Wifi->isConnected())
    {
      this->_Ntp->run();
      this->_OTA->run();
    }
    mqttOSCommands();
}
