#include <Arduino.h>
#include "filemanager.h"
#include "logging.h"
#include "network.h"
#include "led.h"
#include "button.h"
#include "NetworkIdent.h"
#include "../lib/network/webSrc/setupPage.h"


LED wifiLed(D1);
LED errorLed(D7);
LED workLed(D2);
Button mainButton(D6, 3);
WiFiManager wifiManager(&wifiLed);
Filemanager FM;
Network test(&FM, &wifiManager);
ErrorHandler mainHandler(wifiManager.getINode(), &errorLed, &workLed);
NetworkIdent networkIdent(&FM, &wifiManager);

void handleTest()
{
   ESP8266WebServer* webserver = test.getWebserver();

   webserver->send(200, "text/plain", "lololol");

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

void setup() {
  Serial.begin(921600);
  Serial.println("Start");
  Serial.println("");

  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  WiFi.stopSmartConfig();
  workLed.ledOn();
  wifiManager.setClassName("wifiManager");
  test.setClassName("network");
  mainHandler.addNewNode(test.getINode(), "network");
  mainHandler.addNewNode(networkIdent.getINode(), "NetworkIdent");
  
  //Serial.println(mainHandler.verifyAmountOfNodes());
  delay(2000);
  FM.mount();
  FM.getSerialFileStructure();
  test.begin();
  test.addService("/new", handleTest);

}

void loop() {  
  test.run();

  wifiLed.run();
  errorLed.run();
  workLed.run();
  wifiManager.run();
  errorHandle();
  getPerformance();
  networkIdent.loop();

  if(wifiManager.getWiFiState() == WL_CONNECTED)
  {
    static ulong lastCall = 0;
    int delay = 3000;
    static int count = 0;

    if(count < 5 && millis() >= (lastCall + delay))
    {
      lastCall = millis();
      logger logging;
      logging.SFLog("main", "main", "Send UDP!");

      networkIdent.searchForService("Webserver");
      count++;
    }
    
  }

}