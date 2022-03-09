#include <Arduino.h>
#include "wrapper.h"

#include "bme280/bme280.h"


//MAIN OS
espOS* mainOS;

//Needed Objects
Network* _Network;
MQTTHandlerV2* _MQTT;
Filemanager* _FM;

//TEMP Module
BME280* temp;

void setup() { 
  mainOS = new espOS();

//Get needed Objects
 _Network = mainOS->getNetworkManagerObj();
 _MQTT = mainOS->getMqttHandler();
 _FM = mainOS->getFilemanagerObj();

  temp = new BME280(_Network, _MQTT, _FM);
  //start espOS
  mainOS->begin();

  temp->begin();
}

void loop() {
  //run Services
  mainOS->run();

  temp->run();
}