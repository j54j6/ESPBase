#include "dht11Temp.h"

dht11Temp::dht11Temp(int dhtPin, bool isFahrenheit)
{
    dht = new DHT(dhtPin, DHT11);
    this->fahrenheit = isFahrenheit;
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

}