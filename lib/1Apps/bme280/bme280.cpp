#include "bme280.h"

BME280::BME280(Network* network, MQTTHandlerV2* mqtt, Filemanager* _FM)
{
    #ifdef compileLoggingClass
        this->logger = new SysLogger("BME280");
    #endif
    this->mqtt = mqtt;
    this->network = network;
    this->_FM = _FM;
    this->_FM->begin();
}

boolean BME280::loadConfig()
{
    if(this->_FM->fExist(BME280Config))
    {
        if(this->_FM->checkForKeyInJSONFile(BME280Config, "httpEnabled"))
        {
            this->httpServiceEnabled = this->_FM->returnAsBool(this->_FM->readJsonFileValueAsString(BME280Config, "httpEnabled").c_str());
        }
        else
        {
            if(this->_FM->appendJsonKey(BME280Config, "httpEnabled", defaultBME280ClassHTTPServiceEnabled))
            {
                this->httpServiceEnabled = defaultBME280ClassHTTPServiceEnabled;
                #ifdef compileLoggingClass
                    this->logger->logIt("loadConfig", "Key 'httpEnabled' added to Config!", 3);
                #endif
            }
            else
            {
                this->httpServiceEnabled = defaultBME280ClassHTTPServiceEnabled;
            }
        }

        if(this->_FM->checkForKeyInJSONFile(BME280Config, "mqttEnabled"))
        {
            this->mqttServiceEnabled = this->_FM->returnAsBool(this->_FM->readJsonFileValueAsString(BME280Config, "mqttEnabled").c_str());
        }
        else
        {
            if(this->_FM->appendJsonKey(BME280Config, "mqttEnabled", defaultBME280ClassMQTTServiceEnabled))
            {
                this->mqttServiceEnabled = defaultBME280ClassMQTTServiceEnabled;
                #ifdef compileLoggingClass
                    this->logger->logIt("loadConfig", "Key 'mqttEnabled' added to Config!", 3);
                #endif
            }
            else
            {
                this->mqttServiceEnabled = defaultBME280ClassMQTTServiceEnabled;
            }
        }

        if(this->_FM->checkForKeyInJSONFile(BME280Config, "useHTTPS"))
        {
            this->httpsEnabled = this->_FM->returnAsBool(this->_FM->readJsonFileValueAsString(BME280Config, "useHTTPS").c_str());
        }
        else
        {
            if(this->_FM->appendJsonKey(BME280Config, "useHTTPS", defaultBME280ClassHTTPUseHTTPSEnabled))
            {
                this->httpsEnabled = defaultBME280ClassHTTPUseHTTPSEnabled;
                #ifdef compileLoggingClass
                    this->logger->logIt("loadConfig", "Key 'useHTTPS' added to Config!", 3);
                #endif
            }
            else
            {
                this->httpsEnabled = defaultBME280ClassHTTPUseHTTPSEnabled;
            }
        }

        //MQTT Topic
        if(this->_FM->checkForKeyInJSONFile(BME280Config, "mqttTopic"))
        {
            this->mqttTopic = this->_FM->readJsonFileValueAsString(BME280Config, "mqttTopic");
        }
        else
        {
            if(this->_FM->appendJsonKey(BME280Config, "mqttTopic", String(WiFi.macAddress() + "/").c_str()))
            {
                this->mqttTopic = WiFi.macAddress() + "/"; //Default mac
                #ifdef compileLoggingClass
                    this->logger->logIt("loadConfig", "Key 'mqttTopic' added to Config!", 3);
                #endif
            }
            else
            {
                this->mqttTopic = WiFi.macAddress() + "/";
            }
        }

        //check intervall
        if(this->_FM->checkForKeyInJSONFile(BME280Config, "checkIntervall"))
        {
            this->checkIntervall = this->_FM->readJsonFileValueAsString(BME280Config, "checkIntervall").toInt();
        }
        else
        {
            if(this->_FM->appendJsonKey(BME280Config, "checkIntervall", String(defaultBME280ClassCheckIntervall).c_str()))
            {
                this->checkIntervall = defaultBME280ClassCheckIntervall; //Default mac
                #ifdef compileLoggingClass
                    this->logger->logIt("loadConfig", "Key 'checkIntervall' added to Config!", 3);
                #endif
            }
            else
            {
                this->checkIntervall = defaultBME280ClassCheckIntervall;
            }
        }

        //retain messages
        if(this->_FM->checkForKeyInJSONFile(BME280Config, "retain"))
        {
            this->retainMessages = this->_FM->returnAsBool(this->_FM->readJsonFileValueAsString(BME280Config, "retain").c_str());
        }
        else
        {
            if(this->_FM->appendJsonKey(BME280Config, "retain", "true"))
            {
                this->retainMessages = true;
                #ifdef compileLoggingClass
                    this->logger->logIt("loadConfig", "Key 'retain' added to Config!", 3);
                #endif
            }
            else
            {
                this->retainMessages = true;
            }
        }
        return true;
    }
    else
    {
        #ifdef compileLoggingClass
            this->logger->logIt("loadConfig", "No Config file found! - create it!");
        #endif
        if(this->_FM->createFile(BME280Config))
        {
            #ifdef compileLoggingClass
                this->logger->logIt("loadConfig", "Config file successfully created!");
            #endif
            return loadConfig();
        }
        else
        {
            #ifdef compileLoggingClass
                this->logger->logIt("loadConfig", "Error while creating Configfile!", 5);
            #endif
            return false;
        }
    }
    return false;
}

void BME280::begin()
{
    loadConfig();
    boolean beginSuccessfull = bme.begin(0x76);

    if(beginSuccessfull)
    {
        #ifdef compileLoggingClass
            this->logger->logIt("begin", "BME successfully initiated!");
        #endif
        this->beginSuccessfull = true;
        this->readValues();
    }
    else
    {
        #ifdef compileLoggingClass
            this->logger->logIt("begin", "Error while starting BME280!", 5);
        #endif
    }
    this->webserver = this->network->getWebserver();

    if(this->httpServiceEnabled)
    {
        #ifdef compileLoggingClass
            this->logger->logIt("begin", "HTTP Service enabled!");
        #endif
        addWebservice();
    }
    
    this->network->addModuleConfigWebsite("/bme280", std::bind(&BME280::handleConfigSite, this), "Thermometer", true);
    this->network->addService("/savebme280", std::bind(&BME280::saveNewConfig, this));

}

void BME280::readValues()
{
    this->lastTemperature = bme.readTemperature();
    this->lastHumidity = bme.readHumidity();
    this->lastPressure = bme.readPressure() / 100.0F;
    this->lastAltitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void BME280::addWebservice()
{
    //this->network->addService("/bme280", std::bind(&BME280::handleConfigSite, this));
    if(this->httpServiceEnabled)
    {
        this->network->addService("/bme280/temperature", std::bind(&BME280::sendTemperature, this));
        this->network->addService("/bme280/humidity", std::bind(&BME280::sendHumidity, this));
        this->network->addService("/bme280/pressure", std::bind(&BME280::sendPressure, this));
        this->network->addService("/bme280/altitude", std::bind(&BME280::sendAltitude, this));
        this->network->addService("/bme280/temperature/json", std::bind(&BME280::sendTemperatureJson, this));
        this->network->addService("/bme280/humidity/json", std::bind(&BME280::sendHumidityJson, this));
        this->network->addService("/bme280/pressure/json", std::bind(&BME280::sendPressureJson, this));
        this->network->addService("/bme280/altitude/json", std::bind(&BME280::sendAltitudeJson, this));
        this->network->addService("/bme280/json", std::bind(&BME280::sendDataJson, this));
    }
    else
    {
        logInfo("addWebService", "HTTP Service disabled!")
    }
    
    
}


//HTTP Services

void BME280::sendTemperature()
{
    this->webserver->send(200, "text/plain", String(this->lastTemperature));
}

void BME280::sendTemperatureJson()
{
    String jsonValue = "{\"temperature\" : \"" + String(this->lastTemperature) + "\"}";
    this->webserver->send(200, "application/json", jsonValue);
}

void BME280::sendHumidity()
{
    this->webserver->send(200, "text/plain", String(this->lastHumidity));
}

void BME280::sendHumidityJson()
{
    String jsonValue = "{\"humidity\" : \"" + String(this->lastHumidity) + "\"}";
    this->webserver->send(200, "application/json", jsonValue);
}

void BME280::sendPressure()
{
    this->webserver->send(200, "text/plain", String(this->lastPressure));
}

void BME280::sendPressureJson()
{
    String jsonValue = "{\"pressure\" : \"" + String(this->lastPressure) + "\"}";
    this->webserver->send(200, "application/json", jsonValue);
}

void BME280::sendAltitude()
{
    this->webserver->send(200, "text/plain", String(this->lastAltitude));
}

void BME280::sendAltitudeJson()
{
    String jsonValue = "{\"altitude\" : \"" + String(this->lastAltitude) + "\"}";
    this->webserver->send(200, "application/json", jsonValue);
}

void BME280::sendDataJson()
{
    String jsonAllValue = "{\"temperature\" : \"" + String(this->lastTemperature) + "\", \"humidity\" : \"" + String(lastHumidity) + "\", \"pressure\" : \"" + String(this->lastPressure) + "\", \"altitude\": \"" + String(this->lastAltitude) + "\"}";
    this->webserver->send(200, "application/json", jsonAllValue);
}


void BME280::handleConfigSite()
{
    WebManager* web = this->network->getWebManager();
    DynamicJsonDocument keys(900);

    keys["temperature"] = String(this->lastTemperature);
    keys["humidity"] = String(this->lastHumidity);
    keys["pressure"] = String(this->lastPressure);
    keys["altitude"] = String(this->lastAltitude);
    
    

    if(this->retainMessages)
    {
        keys["mqttRetained"] = "Enabled";
    }
    else
    {
        keys["mqttRetained"] = "Disabled";
    }

    if(this->mqttServiceEnabled)
    {
        keys["mqttEnabled"] = "Enabled";
    }
    else
    {
        keys["mqttEnabled"] = "Disabled";
    }
    
    if(this->httpServiceEnabled)
    {
        keys["httpEnabled"] = "Enabled";
    }
    else
    {
        keys["httpEnabled"] = "Disabled";
    }

    if(this->mqttTopic == "" || this->mqttTopic.length () <= 0)
    {
        keys["mqttTopic"] = "No MQTT Topic defined!";
    }
    else
    {
        keys["mqttTopic"] = String(this->mqttTopic);
    }

    Serial.println(this->checkIntervall);

    if(this->checkIntervall == NULL || String(this->checkIntervall) == "" || String(this->checkIntervall) == " " || String(this->checkIntervall).length() <= 0)
    {
        keys["intervall"] = "No Intervall defined!";
    }
    else
    {
        keys["intervall"] = String(this->checkIntervall);
    }

    web->sendWebsiteWithProcessor("config/os/web/bme280.html", keys);
}

void BME280::saveNewConfig()
{
    boolean error = false;
    logInfo("saveNewConfig", "Try saving sended Config...")

    if(this->webserver->hasArg("mqttEnabled") && this->webserver->arg("mqttEnabled") != "" && this->webserver->arg("mqttEnabled") != " " && (this->webserver->arg("mqttEnabled") == "true" || this->webserver->arg("mqttEnabled") == "false"))
    {
        if(!this->_FM->changeJsonValueFile(BME280Config, "mqttEnabled", this->webserver->arg("mqttEnabled").c_str()))
        {
            error = true;
        }
    }

    if(this->webserver->hasArg("httpEnabled") && this->webserver->arg("httpEnabled") != "" && this->webserver->arg("httpEnabled") != " " && (this->webserver->arg("httpEnabled") == "true" || this->webserver->arg("httpEnabled") == "false"))
    {
        if(!this->_FM->changeJsonValueFile(BME280Config, "httpEnabled", this->webserver->arg("httpEnabled").c_str()))
        {
            error = true;
        }
    }

    if(this->webserver->hasArg("mqttRetainedInp") && this->webserver->arg("mqttRetainedInp") != "" && this->webserver->arg("mqttRetainedInp") != " " && (this->webserver->arg("mqttRetainedInp") == "true" || this->webserver->arg("mqttRetainedInp") == "false"))
    {
        if(!this->_FM->changeJsonValueFile(BME280Config, "mqttRetainedInp", this->webserver->arg("mqttRetainedInp").c_str()))
        {
            error = true;
        }
    }

    if(this->webserver->hasArg("mqttTopic") && this->webserver->arg("mqttTopic") != "" && this->webserver->arg("mqttTopic") != " ")
    {
        if(!this->_FM->changeJsonValueFile(BME280Config, "mqttTopic", this->webserver->arg("mqttTopic").c_str()))
        {
            error = true;
        }
    }

    if(this->webserver->hasArg("checkIntervall") && this->webserver->arg("checkIntervall") != "" && this->webserver->arg("checkIntervall") != " ")
    {
        if(!this->_FM->changeJsonValueFile(BME280Config, "checkIntervall", this->webserver->arg("checkIntervall").c_str()))
        {
            error = true;
        }
    }

    if(error)
    {
        logError("saveNewConfig", "Error while saving new Config to FS!")
    }

    this->webserver->sendHeader("Location", "/bme280",true);
    this->webserver->send(302, "text/plane","");
    this->begin();
}

void BME280::sendMQTTMessage()
{
    if(this->network->isWiFiConnected() && this->mqtt->isConnected())
    {
        logTrace("SendMQTTMessage", "Send Data via MQTT")

        //temperature
        String temperatureTopic = this->mqttTopic + "temperature";

        boolean tempSend = this->mqtt->publish(temperatureTopic.c_str(), String(this->lastTemperature).c_str(), this->retainMessages);

        //Humidity
        String humidTopic = this->mqttTopic + "humidity";
        boolean humidSend = this->mqtt->publish(humidTopic.c_str(), String(this->lastHumidity).c_str(), this->retainMessages);
        
        //Altitude
        String altTopic = this->mqttTopic + "altitude";
        boolean altSend = this->mqtt->publish(altTopic.c_str(), String(this->lastAltitude).c_str(), this->retainMessages);

        //Pressure
        String pressureTopic = this->mqttTopic + "pressure";
        boolean presSend = this->mqtt->publish(pressureTopic.c_str(), String(this->lastPressure).c_str(), this->retainMessages);

        if(!tempSend || !humidSend || !altSend || !presSend)
        {
            logError("sendMQTTMessage", "Error while sending values!");
        }

    }
}

void BME280::enableMQTTService()
{
    if(!beginMQTTSuccessfull && beginSuccessfull && this->mqttServiceEnabled && this->mqtt->isConnected())
    {
        if(this->mqtt->subscribe(String(this->mqttTopic + "control").c_str()))
        {
            logInfo("enableMQTTService", "MQTT Service enabled - Subscribed to '" + String(this->mqttTopic) + "control" + "'");
            this->beginMQTTSuccessfull = true;
        }
    }
}

void BME280::run()
{
    static unsigned long lastCall = millis();

    if((lastCall + (this->checkIntervall * 0x3E8)) < millis())
    {
        this->readValues();
        lastCall = millis();
        this->sendMQTTMessage();
    }
    
    if(this->network->isWiFiConnected() && !beginMQTTSuccessfull)
    {
        this->enableMQTTService();
    }
}