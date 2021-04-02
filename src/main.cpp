#include <Arduino.h>
#include "wrapper.h"
#include "dht11Temp.h"

//ESPOS
espOS mainOS(D2, D7, D1);


//Module Object
dht11Temp* _dht11;

//dependent dht11 to OS Objects
Filemanager* _FM;
MQTTHandler* _mqttHandler;


void setup()
{ 
  //get dependent Objects for dht11 Module
  _FM = mainOS.getFilemanagerObj();
  _mqttHandler = mainOS.getMqttHandler();

  //Init DHT11 Module
  _dht11 = new dht11Temp(_FM, _mqttHandler, D5);

  //start espOS
  mainOS.begin();
}

void loop()
{
  //run Services
  mainOS.run();

  //run DHT11 Module
  _dht11->run();
}