#include "dht11Temp.h"

dht11Temp::dht11Temp(MQTTHandler* mqtt, int dhtPin, bool isFahrenheit)
{
    dht = new DHT(dhtPin, DHT11);
    this->fahrenheit = isFahrenheit;
    this->mqtt = mqtt;
}

float dht11Temp::getTemp()
{
    return actualTemp;
}

float dht11Temp::getHumidity()
{
    return actualHumid;
}

void dht11Temp::setUseComputedTemp(bool newVal)
{
    this->useComputedTemp = newVal;
}

void dht11Temp::setMQTT(MQTTHandler* mqtt)
{
    this->mqtt = mqtt;
}

String dht11Temp::getTempAndHumidAsJSON()
{
    String output = "{\"humid\" :\"";
    output += this->actualHumid;
    output += "\", \"temp\": \"";
    output += actualTemp;
    output += "\", \"name\": \"";
    output += deviceName;
    output += "\", \"mac\": \"";
    output += WiFi.macAddress();
    output += "\"}";

    Serial.println(output);
    return output;
}

void dht11Temp::mqttCyclicReport()
{
    if(millis() < nextCall)
    {
        return;
    }
    Serial.println("1");
    if(this->mqtt != nullptr && this->mqtt->isConnected())
    {
        Serial.println("2");
        String message = getTempAndHumidAsJSON();
        if(mqtt->publish(this->mqttPath, message.c_str()))
       {
           Serial.println("3");
            nextCall = millis() + this->nextReportDelayIsMs;
        }
        else
        {
            nextCall = millis() + 3000; //Add 5 minutes
        }
    }
    else
    {
        nextCall = millis() + 3000; //Add 5 minutes
    }
    
}

void dht11Temp::run()
{
    if(!this->begin)
    {
        dht->begin();
        this->begin = true;
    }
    float cachedHumid = dht->readHumidity();
    float cachedTemp;

    if(useComputedTemp)
    {
        cachedTemp = dht->computeHeatIndex(false);
    }
    else
    {
        cachedTemp = dht->readTemperature();
    }

    if(!isnan(cachedHumid))
    {
        actualHumid = cachedHumid;
    }
    else
    {
        actualHumid = 100;
    }

    if(!isnan(cachedTemp))
    {
        if(fahrenheit)
        {
            cachedTemp = dht->convertCtoF(cachedHumid);
        }
        actualTemp = cachedTemp;
    }
    mqttCyclicReport();

}