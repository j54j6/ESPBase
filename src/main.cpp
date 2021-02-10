#include <Arduino.h>
#include "wrapper.h"

espOS mainOS;

MQTTHandler *mqtthandler;
WiFiManager *wifiManager;
Filemanager *FM;
OTA_Manager *otaManager;
Network* network;



void function()
{
    static int step = 0;
    static int lastCall = 0;
    static bool readActive = false;
  if(mqtthandler->getCallback()->payload != "")
  {
    Serial.println("-------------MQTT Message-------------");
    Serial.print("Topic: ");
    Serial.println(mqtthandler->getCallback()->topic);
    Serial.print("Message: ");
    Serial.println(mqtthandler->getCallback()->payload);

    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "restart")
    {
      ESP.restart();
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "shutdown")
    {
      ESP.deepSleep(5000);
    }
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "sayWhat")
    {
      mqtthandler->publish("/home/public", "heyho^^");
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
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "readFile" || readActive == true)
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
    if(strcmp(mqtthandler->getCallback()->topic, "home/control") == 0 && mqtthandler->getCallback()->payload == "update")
    {
      //otaManager.test();
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

    mqtthandler->getCallback()->reset();
  }


  if(network->getDeviceIsConfigured())
  {
    if(wifiManager->isConnected())
    {
      if(step > 1 & !mqtthandler->isConnected())
      {
        Serial.println("MQTT Broker - Connection Lost - reconnect!");
        step = 0;
      }

      switch(step) {
        case 0:
        
          Serial.println("Connect with MQTT");
          
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
            lastCall = millis();
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
}

void loop()
{
  mainOS.run();
  function(); 
}