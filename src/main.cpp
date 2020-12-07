#include <Arduino.h>
#include "filemanager.h"
#include "logging.h"
#include "network.h"
#include "led.h"
#include "button.h"
#include "serviceHandler.h"
#include "../lib/network/webSrc/setupPage.h"
#include "wifiManager.h"
#include "mqttHandler.h"

LED wifiLed(D1);
LED errorLed(D7);
LED workLed(D2);
Button mainButton(D6, 3);
WiFiManager wifiManager(&wifiLed);
Filemanager FM;
Network test(&FM, &wifiManager);
ErrorHandler mainHandler(wifiManager.getINode(), &errorLed, &workLed);
ServiceHandler networkIdent(&FM, &wifiManager);
udpManager udpManage(&wifiManager, 63547);
MQTTHandler mqtthandler(&FM, &wifiManager, &networkIdent);


void handleTest()
{
   ESP8266WebServer* webserver = test.getWebserver();

   webserver->send(200, "text/plain", "Das ist eine Testnachricht");
   
}


void errorHandle()
{
  mainHandler.checkForError();  
}

void getPerformance()
{
  int outputDelay = 30000;
  static ulong lastCall = millis();
  ulong wifiPerformace = wifiManager.getCallPerSecond();
  ulong networkPerformance = test.getCallPerSecond();

  if(millis() >= (lastCall + outputDelay))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "\nWiFi-Manager: ";
      message += wifiPerformace;
      message += "x/s \n";
      message += "WiFi-State: ";
      message += wifiManager.getWiFiState();
      message += "\nNetwork: ";
      message += networkPerformance;
      message += "x/s";
      logging.SFLog("Main", "getPerformance", message.c_str(), 0);
    #endif
    lastCall = millis();
  }
}

void getMqtt(char* topic, byte* payload, unsigned int length)
{
  Serial.println("------------------");
  Serial.println("Message received!");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("length: ");
  Serial.println(length);
  
}



void setup() {
  Serial.begin(921600);
  //Disable WifiAutoConnect and onboard WifiConfig
  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.stopSmartConfig();

  //default enable workLed
  workLed.ledOn();

  //define className shown in ErrorHandler
  wifiManager.setClassName("wifiManager");
  test.setClassName("network");

  //add modules to dedicated ErrorHandler
  mainHandler.addNewNode(test.getINode(), "network");
  mainHandler.addNewNode(networkIdent.getINode(), "NetworkIdent");
  mainHandler.addNewNode(mqtthandler.getINode(), "mqtt");
  
  //Serial.println(mainHandler.verifyAmountOfNodes());

  //preMount Filesystem
  FM.mount();

  //get Filestructure - only for dev
  FM.getSerialFileStructure();

  //start Network
  test.begin();

  //add webservice to webserver@Network
  //test.addService("/new", handleTest);
  //test.startWebserver(80);

  //start Listening on UDP-NetworkIdentPort
  networkIdent.beginListen();

  networkIdent.addService("NetworkIdent", "63547");
  networkIdent.addService(true, false, "mqttConfigServer", "1883", IPAddress(192,168,178,27));
  networkIdent.addService(false, false, "mqtt", "1883", IPAddress(192,168,178,27));
  //MQTT
  Serial.println("main1");
  IPAddress mqserv = IPAddress(192,168,178,27);
  bool mqt = mqtthandler.setServer(mqserv, 1883);

  Serial.print("mqt set Server: ");
  Serial.println(mqt);
//  mqtthandler.setCallback(getMqtt);
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

  //dedicated ErrorHandler
  errorHandle();

  //performanceControl
  getPerformance();

  //NetworkIdent
  networkIdent.loop();
  static int step = 0;
  static int lastCall = 0;


  mqtthandler.run();

  if(strcmp(mqtthandler.getCallback()->payload, "") != 0)
  {
    Serial.println("-------------MQTT Message-------------");
    Serial.print("Topic: ");
    Serial.println(mqtthandler.getCallback()->topic);
    Serial.print("Message: ");
    Serial.println(mqtthandler.getCallback()->payload);

    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && strcmp(mqtthandler.getCallback()->payload, "reset") == 0)
    {
      errorLed.ledOn();
    }
    if(strcmp(mqtthandler.getCallback()->topic, "home/control") == 0 && strcmp(mqtthandler.getCallback()->payload, "off") == 0)
    {
      errorLed.ledOff();
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
          Serial.println("Connect with MQTT");
          if(mqtthandler.connect())
          {
            Serial.println("Successfully loaded Connect Config!");
            step++;
            break;
          }
          else
          {
            Serial.println("Error connectng with MQTT Broker!");
          }
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