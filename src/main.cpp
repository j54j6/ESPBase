#include <Arduino.h>
#include "filemanager.h"
#include "logging.h"
#include "network.h"
#include "led.h"
#include "button.h"
#include "NetworkIdent.h"
#include "externServiceHandler.h"
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

udpManager udpManage(&wifiManager, 63547);

ExternServiceHandler extServices(&FM, &wifiManager, &networkIdent);

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
  
  //Serial.println(mainHandler.verifyAmountOfNodes());

  //preMount Filesystem
  FM.mount();

  //get Filestructure - only for dev
  FM.getSerialFileStructure();

  //start Network
  test.begin();

  //add webservice to webserver@Network
  test.addService("/new", handleTest);
  test.startWebserver(80);

  //start Listening on UDP-NetworkIdentPort
  networkIdent.beginListen();

  networkIdent.addService("NetworkIdent", "63547");
}

void loop() {
  //Button
  ButtonClicks button = mainButton.run();

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
  //getPerformance();


  //NetworkIdent
  networkIdent.loop();

  //external Service Handler
  extServices.loop();
}