#include "dht11Temp.h"

dht11Temp::dht11Temp(Filemanager* _FM, MQTTHandler* mqtt, int dhtPin, bool isFahrenheit)
{
    dht = new DHT(dhtPin, DHT11);
    this->fahrenheit = isFahrenheit;
    this->mqtt = mqtt;
    this->_FM = _FM;
}

bool dht11Temp::createConfigFile()
{
    _FM->createFile(this->configFile);
    String mqttpath = "espos/usedata/";
    mqttpath += WiFi.macAddress();
    mqttpath += "/module/dht11Temp/temperature";
    String devicename = WiFi.macAddress();
    String mqttControlPath = "espOS/devices/" + WiFi.macAddress() + "/module/dht11/control";
    bool bpath = _FM->appendJsonKey(this->configFile, "mqttPath", mqttpath.c_str());
    bool bdeviceName = _FM->appendJsonKey(this->configFile, "deviceName", devicename.c_str());
    bool bfahrenheit = _FM->appendJsonKey(this->configFile, "isFahrenheit", "false");
    bool bcompute = _FM->appendJsonKey(this->configFile, "useComputedTemp", "true");
    bool bCheckIntervall = _FM->appendJsonKey(this->configFile, "checkIntervall", "120");
    bool bmqttcontrolPath = _FM->appendJsonKey(this->configFile, "controlPath", mqttControlPath.c_str());

    if(bpath && bdeviceName && bfahrenheit && bcompute && bCheckIntervall && bmqttcontrolPath)
    {
        logging.logIt("createConfigFile", "Successfully created config File");
        return true;
    }
    else
    {
        logging.logIt("createConfigFile", "Can't create Config File! - results: Path: " + String(bpath) + ", deviceName: " + String(bdeviceName) + ", isFahrenheit: " + String(bfahrenheit) + ", Compute: " + String(bcompute) + ", CheckIntervall: " + String(bCheckIntervall) + ", MQTT Controlpath: " + String(bmqttcontrolPath));
        return false;
    }
}

bool dht11Temp::begin()
{
    if(!_FM->fExist(this->configFile))
    {
        if(!createConfigFile())
        {
            logging.logIt("begin", "Can't create Configfile!");
            return false;
        }
    }
    else
    {
        bool res = true;
        logging.logIt("begin", "Config File exist! - read Config...",3);
        if(_FM->checkForKeyInJSONFile(configFile, "useComputedTemp"))
        {
            this->useComputedTemp = _FM->returnAsBool(_FM->readJsonFileValue(this->configFile, "useComputedTemp"));
        }
        else
        {
            logging.logIt("begin", "Can't find JSON Key 'useComputedTemp' in ConfigFile! - use default Value", 4);
            this->useComputedTemp = true;
        }
        if(_FM->checkForKeyInJSONFile(configFile, "mqttPath"))
        {
            this->mqttPath = _FM->readJsonFileValue(this->configFile, "mqttPath");
            res = this->mqtt->subscribe(mqttPath.c_str());
        }
        else
        {
            logging.logIt("begin", "Can't find JSON Key 'mqttPath' in ConfigFile! - use default Value", 4);
            String mqttpath = "espos/usedata/";
            mqttpath += WiFi.macAddress();
            mqttpath += "/module/dht11Temp/temperature";
            this->mqttPath = mqttpath;
        }
        if(_FM->checkForKeyInJSONFile(configFile, "deviceName"))
        {
            this->deviceName = _FM->readJsonFileValue(this->configFile, "deviceName");
        }
        else
        {
            logging.logIt("begin", "Can't find JSON Key 'deviceName' in ConfigFile! - use default Value", 4);
            this->deviceName = WiFi.macAddress();
        }
        if(_FM->checkForKeyInJSONFile(configFile, "isFahrenheit"))
        {
            this->fahrenheit = _FM->returnAsBool(_FM->readJsonFileValue(this->configFile, "isFahrenheit"));
        }
        else
        {
            logging.logIt("begin", "Can't find JSON Key 'isFahrenheit' in ConfigFile! - use default Value", 4);
            this->fahrenheit = false;
        }
        if(_FM->checkForKeyInJSONFile(configFile, "checkIntervall"))
        {
            this->tempCheckIntervall = int(_FM->readJsonFileValue(this->configFile, "checkIntervall"));
        }
        else
        {
            logging.logIt("begin", "Can't find JSON Key 'checkIntervall' in ConfigFile! - use default Value", 4);
            this->tempCheckIntervall = 300;
        }
        if(_FM->checkForKeyInJSONFile(configFile, "controlTopic"))
        {
            this->controlTopic = int(_FM->readJsonFileValue(this->configFile, "controlTopic"));
        }
        else
        {
            logging.logIt("begin", "Can't find JSON Key 'controlTopic' in ConfigFile! - use default Value", 4);
            this->controlTopic = "espOS/devices/" + WiFi.macAddress() + "/module/dht11/control";
        }
        res = this->mqtt->subscribe(this->controlTopic.c_str());
        if(res)
        {
            logging.logIt("begin", "Subscribed to Controltopic " + String(controlTopic));
        }
        if(this->mqttPath != "" && this->deviceName != "" && this->tempCheckIntervall > 0 && res == true)
        {
            logging.logIt("begin", "Module configured!");
            logging.logIt("begin", "Subscribed to controlTopic");
            
            return true;
        }
    }
    return false;
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

    //Serial.println(output);
    return output;
}

bool dht11Temp::mqttGetemperaturePerRequest(bool cyclicRequest)
{
    bool res = false;
    if(this->mqtt->getCallback()->topic == this->controlTopic || cyclicRequest)
    {
        String command = this->mqtt->getCallback()->payload;
        command.toLowerCase();
        if(command == "readjson")
        {
            logging.logIt("mqttGetemperaturePerRequest", "Read Temp and Humid");
            this->run(true);
            res = this->mqtt->publish(this->mqttPath.c_str(), getTempAndHumidAsJSON().c_str(), true);
        }
        if(command == "readhumid" || cyclicRequest)
        {
            logging.logIt("mqttGetemperaturePerRequest", "Read Humid");
            this->run(true);
            this->mqtt->publish(this->mqttPath.c_str(), String(this->actualHumid).c_str());
            String secondTopic = this->mqttPath + "/humid";
            res = this->mqtt->publish(secondTopic.c_str(), String(this->actualHumid).c_str(), true);

        }
        if(command == "readtemp" || cyclicRequest)
        {
            logging.logIt("mqttGetemperaturePerRequest", "Read Temp");
            this->run(true);
            this->mqtt->publish(this->mqttPath.c_str(), String(this->actualTemp).c_str());
            String secondTopic = this->mqttPath + "/temp";
            res = this->mqtt->publish(secondTopic.c_str(), String(this->actualTemp).c_str(), true);
        }
        return res;
    }
    return true;  
}

void dht11Temp::mqttCyclicReport()
{
    if(millis() < nextCall)
    {
        return;
    }
    if(this->mqtt != nullptr && this->mqtt->isConnected())
    {
        String message = getTempAndHumidAsJSON();
        if(mqtt->publish(this->mqttPath.c_str(), message.c_str()) && mqttGetemperaturePerRequest(true), true)
        {
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

void dht11Temp::run(bool forceRead)
{
    if(!this->classBegin)
    {
        dht->begin();
        if(begin())
        {
            this->classBegin = true;
        }
        
    }

    static ulong nextCall = 0;

    if(millis() >= nextCall || forceRead)
    {
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

        nextCall = millis() + (this->tempCheckIntervall*1000);
    }
    if(!forceRead)
    {
        mqttGetemperaturePerRequest();
        mqttCyclicReport();
    }
    

}