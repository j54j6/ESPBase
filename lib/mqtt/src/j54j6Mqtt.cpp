#include "j54j6Mqtt.h"

//extra Stuff
bool J54J6_MQTT::configCheck()
{
    if(FM->fExist(configFile))
    {
        return true;
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "configFile already exist - SKIP!");
        #endif
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "configFile don't exist - try to create");
        #endif

        if(!FM->createFile(configFile))
        {
            #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "Can't create File", 2);
            #endif
            error.error = true;
            error.ErrorCode = 321;
            error.message = "Can't create Config File!";
            error.priority = 5;
            return false;
        }
        
        if(!FM->writeJsonFile(configFile, configFallback, 0))
        {
            #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "configCheck", "Can't write in File", 2);
            #endif
            error.error = true;
            error.ErrorCode = 322;
            error.message = "Can't write in File!";
            error.priority = 5;
            return false;
        }
        #ifdef J54J6_LOGGING_H
            logging.SFLog(className, "configCheck", "Config check successfull");
        #endif
        return true;
    }
}


//Constructor

J54J6_MQTT::J54J6_MQTT(Filemanager* FM, WiFiManager* wifiManager, ServiceHandler* serviceHandler)
{
    this->FM = FM;
    this->wifiManager = wifiManager;
    this->services = serviceHandler;
}


bool J54J6_MQTT::setServer(IPAddress ip, uint16 port, bool save, bool asFallback)
{
    mqttHandlerClient = mqttHandlerClient.setServer(ip, port);

    if(save && !asFallback && services->checkForService(services->getExternalServiceFilename("mqtt").c_str()) == 0 || services->checkForService(services->getExternalServiceFilename("mqtt").c_str()) == 2)
    {
        bool ipChanged = services->changeConfigValue(services->getExternalServiceFilename("mqtt").c_str(), "ip", ip.toString().c_str(), false);
        bool portChanged = services->changeConfigValue(services->getExternalServiceFilename("mqtt").c_str(), "ip", ip.toString().c_str(), false);
        if(ipChanged && portChanged)
        {
            return true;
        }
        else
        {
            String message = "Error while saving the Serveraddress - Results: \n";
            message = "IP: ";
            message += ipChanged;
            message += "\n Port: ";
            message += portChanged;
            logging.SFLog(className, "setServer", message.c_str(), 2);
            return false;
        }
    }
    else if (save && asFallback && services->checkForService(services->getExternalServiceFilename("mqtt", true).c_str()) == 0 || services->checkForService(services->getExternalServiceFilename("mqtt", true).c_str()) == 2)
    {
        bool ipChanged = services->changeConfigValue(services->getExternalServiceFilename("mqtt").c_str(), "ip", ip.toString().c_str(), true);
        bool portChanged = services->changeConfigValue(services->getExternalServiceFilename("mqtt").c_str(), "ip", ip.toString().c_str(), true);
        if(ipChanged && portChanged)
        {
            return true;
        }
        else
        {
            String message = "Error while saving the Serveraddress - Results: \n";
            message = "IP: ";
            message += ipChanged;
            message += "\n Port: ";
            message += portChanged;
            logging.SFLog(className, "setServer - FallbackPart", message.c_str(), 2);
            return false;
        }
    }
    else
    {
        return true;
    }
    return false;
}

void J54J6_MQTT::setCallback(MQTT_CALLBACK_SIGNATURE)
{
    mqttHandlerClient = mqttHandlerClient.setCallback(callback);
}

void J54J6_MQTT::setClient(Client& client)
{
    mqttHandlerClient = mqttHandlerClient.setClient(client);
}

void J54J6_MQTT::setStream(Stream& stream)
{
    mqttHandlerClient = mqttHandlerClient.setStream(stream);
}

void J54J6_MQTT::setKeepAlive(uint16_t keepAlive)
{
    mqttHandlerClient = mqttHandlerClient.setKeepAlive(keepAlive);
}

void J54J6_MQTT::setSocketTimeout(uint16_t timeout)
{
    mqttHandlerClient = mqttHandlerClient.setSocketTimeout(timeout);
}

bool J54J6_MQTT::setBufferSize(uint16_t size)
{
    return mqttHandlerClient.setBufferSize(size);
}

uint16_t J54J6_MQTT::getBufferSize()
{
    return mqttHandlerClient.getBufferSize();
}


/*
    Set Stuff for Module Conf
*/
bool J54J6_MQTT::setId(const char* id)
{
    if(strcmp(id, "") == 0)
    {
        logging.SFLog(className, "setID", "ID can't be NULL - return false", 1);
        return false;
    }

    if(FM->checkForKeyInJSONFile(configFile, "id"))
    {
        return FM->changeJsonValueFile(configFile, "id", id);
    }
    else
    {
        return FM->appendJsonKey(configFile, "id", id);
    }
    return false;
}

bool J54J6_MQTT::setUser(const char* user)
{
    if(strcmp(user, "") == 0)
    {
        logging.SFLog(className, "setUser", "user can't be NULL - return false", 1);
        return false;
    }

    if(FM->checkForKeyInJSONFile(configFile, "user"))
    {
        return FM->changeJsonValueFile(configFile, "user", user);
    }
    else
    {
        return FM->appendJsonKey(configFile, "user", user);
    }
    return false;
}

bool J54J6_MQTT::setPass(const char* pass)
{
    if(strcmp(pass, "") == 0)
    {
        logging.SFLog(className, "setPass", "Pass can't be NULL - return false", 1);
        return false;
    }

    if(FM->checkForKeyInJSONFile(configFile, "pass"))
    {
        return FM->changeJsonValueFile(configFile, "pass", pass);
    }
    else
    {
        return FM->appendJsonKey(configFile, "pass", pass);
    }
    return false;
}

bool J54J6_MQTT::setWillTopic(const char* willTopic)
{
    if(strcmp(willTopic, "") == 0)
    {
        logging.SFLog(className, "willTopic", "willTopic can't be NULL - return false", 1);
        return false;
    }

    if(FM->checkForKeyInJSONFile(configFile, "willTopic"))
    {
        return FM->changeJsonValueFile(configFile, "willTopic", willTopic);
    }
    else
    {
        return FM->appendJsonKey(configFile, "willTopic", willTopic);
    }
    return false;
}

bool J54J6_MQTT::setWillQos(uint8_t willQos)
{
    if(FM->checkForKeyInJSONFile(configFile, "willQos"))
    {
        return FM->changeJsonValueFile(configFile, "willQos", String(willQos).c_str());
    }
    else
    {
        return FM->appendJsonKey(configFile, "willQos", String(willQos).c_str());
    }

    return false;
}

bool J54J6_MQTT::setWillRetain(bool willRetain)
{
    if(FM->checkForKeyInJSONFile(configFile, "willRetain"))
    {
        return FM->changeJsonValueFile(configFile, "willRetain", String(willRetain).c_str());
    }
    else
    {
        return FM->appendJsonKey(configFile, "willRetain", String(willRetain).c_str());
    }
    return false;
}

bool J54J6_MQTT::setWillMessage(const char* willMessage)
{
    if(strcmp(willMessage, "") == 0)
    {
        logging.SFLog(className, "setWillMessage", "WillMessage can't be NULL - return false", 1);
        return false;
    }

    if(FM->checkForKeyInJSONFile(configFile, "willMessage"))
    {
        return FM->changeJsonValueFile(configFile, "willMessage", willMessage);
    }
    else
    {
        return FM->appendJsonKey(configFile, "willMessage", willMessage);
    }
    return false;
}

bool J54J6_MQTT::setCleanSession(bool cleanSession)
{
    if(FM->checkForKeyInJSONFile(configFile, "cleanSession"))
    {
        return FM->changeJsonValueFile(configFile, "cleanSession", String(cleanSession).c_str());
    }
    else
    {
        return FM->appendJsonKey(configFile, "cleanSession", String(cleanSession).c_str());
    }
    return false;
}

/*
    Connect
*/

bool J54J6_MQTT::connect()
{
    logging.SFLog(className, "connect - AUTO", "Try to connect to MQTT Broker - read Config", 0);

    if(!services->checkForService("mqtt", true))
    {
        logging.SFLog(className, "connect - AUTO", "No MQTT Service defined - try to connect to in Object defined MQTT Server", 1);
        if(mqttHandlerClient.connect(String(wifiManager->getDeviceMac()).c_str())) //check if any other connection is saved in Object e.g via setServer()
        {
            logging.SFLog(className, "connect - AUTO", "Successfully connected to in Object defined MQTT Server - return true", 0);
            return true;
        }
        else
        {
            logging.SFLog(className, "connect - AUTO", "Can't connect to MQTT Broker - Maybe no Address set?", 0);
            return false;
        }
    }
    else //MQTT Service defined
    {
        bool setServerSuccess = false;
        bool connectFailed = false;
        for(int i = 0; i <= 1; i++)
        {
            if(i == 0)
            {
                logging.SFLog(className, "connect - AUTO", "Choose Main MQTT Broker", 0);
                mqttHandlerClient = mqttHandlerClient.setServer(services->getServiceIP("mqtt"), services->getServicePort("mqtt"));
                setServerSuccess = true;
            }
            else
            {
                if(FM->fExist(services->getExternalServiceFilename("mqtt", true).c_str()))
                {
                    logging.SFLog(className, "connect - AUTO", "Choose Fallback MQTT Broker", 0);
                    mqttHandlerClient = mqttHandlerClient.setServer(services->getServiceIP("mqtt", true), services->getServicePort("mqtt", true));
                    setServerSuccess = true;
                }
                else
                {
                    logging.SFLog(className, "connect - AUTO", "Can't find any useable MQTT Broker adresses - please define!", 2);
                    return false;
                }
            }

            if(!FM->fExist(configFile))
            {
                return false;
            }                
        }
    }
}


void J54J6_MQTT::run()
{
    /*
    if(!mqttHandlerClient.loop())
    {
        this->error.error = true;
        this->error.ErrorCode = 365;
        this->error.message = "";
    }
    */
    delay(10); //need to fix several stability problems look at https://github.com/256dpi/arduino-mqtt?utm_source=platformio&utm_medium=piohome#notes
}