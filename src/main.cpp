#include <Arduino.h>
#include "logger.h"
#include "filemanager.h"
#include "logger.h"
#include "network.h"
#include "led.h"
#include "button.h"
#include "serviceHandler.h"
#include "../lib/network/webSrc/setupPage.h"
#include "wifiManager.h"
#include "mqttHandler.h"
#include "moduleState.h"

LED wifiLed(D1);
LED errorLed(D7);
LED workLed(D2);
Button mainButton(D6, 3);
Filemanager FM;
WiFiManager wifiManager(&wifiLed, &FM);
Network test(&FM, &wifiManager);
ServiceHandler networkIdent(&FM, &wifiManager, 30);
udpManager udpManage(&FM, &wifiManager, 63547);
MQTTHandler mqtthandler(&FM, &wifiManager, &networkIdent);
ClassModuleMaster testHandler(&errorLed, &workLed);

void handleTest()
{
   ESP8266WebServer* webserver = test.getWebserver();

   webserver->send(200, "text/plain", "Das ist eine Testnachricht");
   
}

void setup() {
  Serial.begin(921600);
  //Disable WifiAutoConnect and onboard WifiConfig
  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.stopSmartConfig();

  //default enable workLed
  workLed.ledOn();
  
  //preMount Filesystem
  FM.mount();
  //FM.format();
  //get Filestructure - only for dev
  FM.getSerialFileStructure();

  //start Network
  test.begin();

  //start Listening on UDP-NetworkIdentPort
  networkIdent.beginListen();

  testHandler.addModuleSlave(wifiManager.getClassModuleSlave());
  testHandler.addModuleSlave(mqtthandler.getClassModuleSlave());
  testHandler.addModuleSlave(networkIdent.getClassModuleSlave());
  testHandler.addModuleSlave(test.getClassModuleSlave());
  testHandler.addModuleSlave(udpManage.getClassModuleSlave());
}

void loop() {
  //Button

  //LED's
  wifiLed.run();
  errorLed.run();
  workLed.run();

  //Network
  test.run();

  //wifiManager
  wifiManager.run();

  //NetworkIdent
  networkIdent.loop();
  static int step = 0;
  static int lastCall = 0;


  mqtthandler.run();

  //ModuleStateMaster
  testHandler.run();


  if(mqtthandler.getCallback()->payload != "")
  {
    Serial.println("-------------MQTT Message-------------");
    Serial.print("Topic: ");
    Serial.println(mqtthandler.getCallback()->topic);
    Serial.print("Message: ");
    Serial.println(mqtthandler.getCallback()->payload);

    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "reset")
    {
      errorLed.ledOn();
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "off")
    {
      errorLed.ledOff();
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "restart")
    {
      ESP.restart();
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "shutdown")
    {
      ESP.deepSleep(5000);
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "sayWhat")
    {
      mqtthandler.publish("/home/public", "heyho^^");
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "allOff")
    {
      workLed.disable();
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "allOn")
    {
      workLed.enable();
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "format")
    {
      workLed.blink(200, false, 10);
      FM.format();
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "getMac")
    {
      mqtthandler.publish("/home/public", wifiManager.getDeviceMac().c_str());
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && mqtthandler.getCallback()->payload == "delay")
    {
      delay(2000);
    }
    mqtthandler.getCallback()->reset();
  }


  if(test.getDeviceIsConfigured())
  {
    if(wifiManager.isConnected())
    {
      if(step > 1 & !mqtthandler.isConnected())
      {
        Serial.println("MQTT Broker - Connection Lost - reconnect!");
        step = 0;
      }

      switch(step) {
        case 0:
        /*
          Serial.println("Connect with MQTT");
          */
          if(mqtthandler.connect())
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
          if(mqtthandler.isConnected())
          {
            Serial.println("Connected with Broker!");
            step++;
            lastCall = millis();
            mqtthandler.subscribe("home/test");
            mqtthandler.subscribe("home/control");
          }
          break;

        case 2:

          if(mqtthandler.publish("home/test", "ESP works!"))
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