#include <Arduino.h>
#include "wrapper.h"
#include "dht11Temp.h"

espOS mainOS(D2, D7, D1);

MQTTHandler *mqtthandler;
WiFiManager *wifiManager;
Filemanager *FM;
OTA_Manager *otaManager;
Network *network;
voltageDetector *voltage;


void function()
{
    static int step = 0;
    //static int lastCall = 0;
    static bool readActive = false;
    static bool delActive = false;
  if(mqtthandler->getCallback()->payload != "")
  {
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "restart")
    {
      ESP.restart();
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "shutdown")
    {
      ESP.deepSleep(5000000);
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "sayWhat")
    {
      Serial.println("Message published: " + String(mqtthandler->publish("/home/public", "heyho^^")));
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "format")
    {
      FM->format();
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "getMac")
    {
      mqtthandler->publish("/home/public", wifiManager->getDeviceMac().c_str());
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "delay")
    {
      delay(2000);
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "hostname")
    {
      wifiManager->setWiFiHostname("2C-F4-32-79-F3-D3");
    }
    if((strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "readFile") || readActive == true)
    {
      if(readActive)
      {
        String filename = mqtthandler->getCallback()->payload;
        filename.replace(" ", "");
        if(!FM->fExist(filename.c_str()))
        {
          Serial.println("Filename '" + filename + String("' doesn't exist!"));
        }
        else
        {
          Serial.println(FM->readFile(filename.c_str()));
        }
        readActive = false;
        Serial.println("Read Mode disabled!");
        
      }
      else
      {
        readActive = true;
        Serial.println("Read Mode enabled!");
      }
      
    }
    if((strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "delFile") || delActive == true)
    {
      if(delActive)
      {
        String filename = mqtthandler->getCallback()->payload;
        filename.replace(" ", "");
        if(!FM->fExist(filename.c_str()))
        {
          Serial.println("Filename '" + filename + String("' doesn't exist!"));
        }
        else
        {
          FM->fDelete(filename.c_str());
        }
        delActive = false;
        Serial.println("Del Mode disabled!");
        
      }
      else
      {
        delActive = true;
        Serial.println("Del Mode enabled!");
      }
      
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "update")
    {
      otaManager->getUpdatesAutoCred();
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "newFeature")
    {
      Serial.println("New is installed!");
    }

    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "updateCheck")
    {
      otaManager->checkUpdatesAutoCred();
    }
    
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "read")
    {
      Serial.println(FM->readFile("config/mainConfig.json"));
    }

    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "port")
    {
      Serial.println(String("Port modified: ") + String(FM->readJsonFileValue("config/mainConfig.json", "port")).toInt());
      Serial.println(String("Port: ") + FM->readJsonFileValue("config/mainConfig.json", "port"));
    }

    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "write")
    {
        FM->appendJsonKey("config/mainConfig.json", "id", "20200425222132534785498");
        FM->appendJsonKey("config/mainConfig.json", "product", "wiFi-thermometer");
        FM->appendJsonKey("config/mainConfig.json", "updateServer", "192.168.178.27");
        FM->appendJsonKey("config/mainConfig.json", "uri", "/");
        FM->appendJsonKey("config/mainConfig.json", "servertoken", "espWiFiThermometerV1");
        FM->appendJsonKey("config/mainConfig.json", "serverpass", "rtgzi32u45z4u5hbnfdnfbdsi");
        FM->appendJsonKey("config/mainConfig.json", "port", "80");
        FM->appendJsonKey("config/mainConfig.json", "updateSearch", "true");
        FM->appendJsonKey("config/mainConfig.json", "autoUpdate", "true");
        FM->appendJsonKey("config/mainConfig.json", "onUpdate", "false");
        FM->appendJsonKey("config/mainConfig.json", "usedChip", "ESP8266");
        FM->appendJsonKey("config/mainConfig.json", "lastUpdate", "20200425");
        FM->appendJsonKey("config/mainConfig.json", "lastUpdateSearch", "0");
        FM->appendJsonKey("config/mainConfig.json", "locked", "false");
        FM->appendJsonKey("config/mainConfig.json", "producedIn", "2020");
        FM->appendJsonKey("config/mainConfig.json", "softwareVersion", "0.0.1");
        FM->appendJsonKey("config/mainConfig.json", "softwareVersionAvail", "");


    }

    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "remove")
    {
      FM->fDelete("config/mainConfig.json");
    }

    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "changeServer")
    {
      otaManager->setUpdateServer("192.168.178.27", 80, "/");
    }

    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "setCheckIntervall")
    {
      otaManager->setCheckIntervall(24);
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "setCheckIntervall2")
    {
      otaManager->setCheckIntervall(2);
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "init")
    {
      otaManager->init();
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "disableLed")
    {
      mainOS.disableLeds();
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "disableAutoUpdate")
    {
      FM->changeJsonValueFile("config/mainConfig.json", "autoUpdate", "false");
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "enableAutoUpdate")
    {
      FM->changeJsonValueFile("config/mainConfig.json", "autoUpdate", "true");
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "enableConfMode")
    {
      mainOS.getModuleMQTTConfig()->setConfModeEnabled(true);
    }

    //mqtthandler->getCallback()->reset();
  }


  if(network->getDeviceIsConfigured())
  {
    if(wifiManager->isConnected())
    {
      if(step > 1 && !mqtthandler->isConnected())
      {
        Serial.println("MQTT Broker - Connection Lost - reconnect!");
        step = 0;
      }

      switch(step) {
        case 0:
          
          if(mqtthandler->connect())
          {
            Serial.println("Successfully loaded Connect Config!");
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
          if(mqtthandler->isConnected())
          {
            Serial.println("Connected with Broker!");
            step++;
            //lastCall = millis();
            mqtthandler->subscribe("home/test");
            mqtthandler->subscribe("home/control");
          }
          break;

        case 2:
          if(mqtthandler->publish("home/test", "ESP works!"))
          {
            Serial.println("Message successfully published!");
            step++;
          }
          else
          {
            Serial.println("There was an Error while Publishing!");

          }
          break;
      }
    }
  }
}
void handleSite()
{
    ESP8266WebServer* webserver = network->getWebserver();
    webserver->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    webserver->sendHeader("Pragma", "no-cache");
    webserver->sendHeader("Expires", "-1");
    webserver->setContentLength(CONTENT_LENGTH_UNKNOWN);
    // HTML Content
    //webserver->send(200, "image/png", FM->readFile("config/os/web/logo.PNG"));
    String websitePrepared = FM->readFile("config/os/web/status.html");

    String connected;
    if(wifiManager->isConnected() == 1)
    {
      websitePrepared.replace("%WiFiStatusClass%", "textOkay");
      connected = "Connected";
    }
    else{
      websitePrepared.replace("%WiFiStatusClass%", "textError");
      connected = "Not Connected!";
    }
    websitePrepared.replace("%WiFiStatus%", connected);

    if(mqtthandler->isConnected())
    {
      websitePrepared.replace("%MQTTStatusClass%", "textOkay");
      websitePrepared.replace("%MQTTStatus%", "Connected");
    }
    else
    {
      websitePrepared.replace("%MQTTStatusClass%", "textError");
      websitePrepared.replace("%MQTTStatus%", "Not Connected");
    }
    
    if(otaManager->getIsLastUpdateCheckFailed())
    {
      websitePrepared.replace("%UpdaterStatusClass%", "textError");
      websitePrepared.replace("%updaterStatus%", "FAILED!");
    }
    else
    {
      if(otaManager->getIsUpdateAvailiable())
      {
          websitePrepared.replace("%UpdaterStatusClass%", "textWarn");
          websitePrepared.replace("%updaterStatus%", "Update available");
      }
      else
      { 
        websitePrepared.replace("%UpdaterStatusClass%", "textOkay");
        websitePrepared.replace("%updaterStatus%", "Okay");
      }
      
    }

    uint32_t RamUsage = (81920 - ESP.getFreeHeap())/1000;
    float RamInPercent = 81920 - ESP.getFreeHeap();
    RamInPercent = RamInPercent/81920;
    RamInPercent = RamInPercent * 100;
    websitePrepared.replace("%BatteryUseageVolt%", String(voltage->getActualVoltage()));
    websitePrepared.replace("%BatteryUseagePercent%", String(voltage->getPercent()));
    websitePrepared.replace("%RamUseageKB%", String(RamUsage));
    websitePrepared.replace("%RamUseagePercent%", String(RamInPercent));
    
    uint storage = ESP.getFlashChipRealSize();
    uint freeSpace = storage - ESP.getFreeSketchSpace();
    freeSpace = freeSpace / 1000;
    Serial.println("Free Space: " + String(freeSpace));
    Serial.println("ESP total: " + String(storage));
    float FreeSpacePercent = freeSpace * 1000;
    FreeSpacePercent = FreeSpacePercent / storage;
    FreeSpacePercent = FreeSpacePercent * 100;
    
    websitePrepared.replace("%flashUseageMB%", String(freeSpace));
    websitePrepared.replace("%flashUseagePercent%", String(FreeSpacePercent));

    webserver->send(200, "text/html", websitePrepared);
}


void sendSpecCss()
{   
  Serial.println("spec csscalled!");
  ESP8266WebServer* webserver = network->getWebserver();

  File specFile = FM->fdOpen("config/os/web/spec.css", "r");
  if (webserver->streamFile(specFile, "text/css") != specFile.size()) 
  {
    Serial.println("Error while streaming!");
  }
 
  specFile.close();
  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec.css"));
}

void sendspecexpcss()
{
  Serial.println("spec exp called!");
  ESP8266WebServer* webserver = network->getWebserver();

  File specExp = FM->fdOpen("config/os/web/spec-exp.css", "r");
  if (webserver->streamFile(specExp, "text/css") != specExp.size()) 
  {
    Serial.println("Error while streaming!");
  }
 
  specExp.close();

  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec-exp.css"));
}

void sendSpecIcons()
{
  Serial.println("send Spec Icons called!");
  ESP8266WebServer* webserver = network->getWebserver();
  File specIcons = FM->fdOpen("config/os/web/spec-icons.css", "r");
  if (webserver->streamFile(specIcons, "text/css") != specIcons.size()) 
  {
    Serial.println("Error while streaming!");
  }
 
  specIcons.close();


  //webserver->send(200, "text/css", FM->readFile("config/os/web/spec-icons.css"));
}

void sendLogo()
{
  Serial.println("send Logo called!");
  ESP8266WebServer* webserver = network->getWebserver();
  File logo = FM->fdOpen("config/os/web/logo.PNG", "r");
  logo.close();
}

void addWebsite()
{
  network->stopWebserver();
  network->addService("/test", handleSite);
  network->addService("/spec-exp.css", sendspecexpcss);
  network->addService("/spec.css", sendSpecCss);
  network->addService("/spec-icons.css", sendSpecIcons);
  network->addService("/logo.PNG", sendLogo);
  network->addService("/logo", sendLogo);
  if(!network->startWebserver(80))
  {
    Serial.println("Can't start Webserver!");
  }
}
void handleVoltage()
{
  Serial.print("Voltage: ");
  Serial.println(voltage->getActualVoltage());
  Serial.print("Percent: ");
  Serial.println(voltage->getPercent());
}

LED test(-1);
void setup()
{  
  mainOS.begin();
  mqtthandler = mainOS.getMqttHandler();
  FM = mainOS.getFilemanagerObj();
  FM->getSerialFileStructure();
  wifiManager = mainOS.getWiFimanagerObj();
  otaManager = mainOS.getOtaManagerObject();
  network = mainOS.getNetworkManagerObj();
  otaManager->setCheckIntervall(24);
  voltage = mainOS.getVoltageDetectorObj();
  
  test.enable();
}

void loop()
{
  mainOS.run();
  function();
  static ulong lastCall = 0;
  if(millis() > lastCall)
  {
    test.enable();
    handleVoltage();
    lastCall = millis() + 5000;
  }
  static bool websiteAlreadyAdded = false;
  if(wifiManager->isConnected() && !websiteAlreadyAdded)
  {
    Serial.println("Website added!");
    addWebsite();
    websiteAlreadyAdded = true;
  }
}