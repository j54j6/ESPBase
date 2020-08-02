#include <Arduino.h>
#include "filemanager.h"
#include "logging.h"
#include "network.h"
#include "led.h"
#include "button.h"
#include "../lib/network/webSrc/setupPage.h"


LED wifiLed(D1);
LED errorLed(D7);
LED workLed(D2);
Button mainButton(D6, 3);
WiFiManager wifiManager(&wifiLed);
Filemanager FM;
Network test(&FM, &wifiManager);
ErrorHandler mainHandler(wifiManager.getINode(), &errorLed, &workLed);

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
      message += "Network: ";
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

  workLed.ledOn();
  wifiManager.setClassName("wifiManager");
  test.setClassName("network");
  mainHandler.addNewNode(test.getINode(), "network");
  
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
}