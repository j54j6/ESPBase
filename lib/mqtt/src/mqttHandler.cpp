#include "mqttHandler.h"

//extra Stuff
bool MQTTHandler::configCheck()
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

MQTTHandler::MQTTHandler(Filemanager* FM, WiFiManager* wifiManager, ServiceHandler* serviceHandler)
{
    this->FM = FM;
    this->wifiManager = wifiManager;
    this->services = serviceHandler;
    init();
}

/*
    Set Stuff
*/
bool MQTTHandler::setServer(IPAddress ip, uint16 port, bool save, bool asFallback)
{
    mqttHandlerClient.setServer(ip, port);
    if(save & !asFallback & (services->checkForService(services->getExternalServiceFilename("mqtt").c_str()) == 0) || (services->checkForService(services->getExternalServiceFilename("mqtt").c_str()) == 2))
    {
        bool ipChanged = services->changeConfigValue(services->getExternalServiceFilename("mqtt").c_str(), "ip", ip.toString().c_str(), false);
        bool portChanged = services->changeConfigValue(services->getExternalServiceFilename("mqtt").c_str(), "ip", ip.toString().c_str(), false);
        if(ipChanged & portChanged)
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

            error.error = false;
            error.ErrorCode = 232;
            error.message = message.c_str();
            error.priority = 2;
            return false;
        }
    }
    else if (save & asFallback & services->checkForService(services->getExternalServiceFilename("mqtt", true).c_str()) == 0 || services->checkForService(services->getExternalServiceFilename("mqtt", true).c_str()) == 2)
    {
        bool ipChanged = services->changeConfigValue(services->getExternalServiceFilename("mqtt").c_str(), "ip", ip.toString().c_str(), true);
        bool portChanged = services->changeConfigValue(services->getExternalServiceFilename("mqtt").c_str(), "ip", ip.toString().c_str(), true);
        if(ipChanged & portChanged)
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

            error.error = false;
            error.ErrorCode = 232;
            error.message = message.c_str();
            error.priority = 2;
            return false;
        }
    }
    else
    {
        return true;
    }
    return false;
}


void MQTTHandler::setCallback(MQTT_CALLBACK_SIGNATURE)
{
    mqttHandlerClient.setCallback(callback);
}

void MQTTHandler::setClient(Client& client)
{
    mqttHandlerClient.setClient(client);
}

void MQTTHandler::setStream(Stream& stream)
{
   mqttHandlerClient.setStream(stream);
}

void MQTTHandler::setKeepAlive(uint16_t keepAlive)
{
    mqttHandlerClient.setKeepAlive(keepAlive);
}

void MQTTHandler::setSocketTimeout(uint16_t timeout)
{
    mqttHandlerClient = mqttHandlerClient.setSocketTimeout(timeout);
}

bool MQTTHandler::setBufferSize(uint16_t size)
{
    return mqttHandlerClient.setBufferSize(size);
}

uint16_t MQTTHandler::getBufferSize()
{
    return mqttHandlerClient.getBufferSize();
}


/*
    Set Stuff for Module Conf
*/

bool MQTTHandler::setId(const char* id)
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

bool MQTTHandler::setUser(const char* user)
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

bool MQTTHandler::setPass(const char* pass)
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

bool MQTTHandler::setWillTopic(const char* willTopic)
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

bool MQTTHandler::setWillQos(uint8_t willQos)
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

bool MQTTHandler::setWillRetain(bool willRetain)
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

bool MQTTHandler::setWillMessage(const char* willMessage)
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

bool MQTTHandler::setCleanSession(bool cleanSession)
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

bool MQTTHandler::connect()
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
        for(int i = 0; i <= 1; i++)
        {
            if(i == 0)
            {   
                logging.SFLog(className, "connect - AUTO", "Choose Main MQTT Broker", 0);
                this->mqttHandlerClient = this->mqttHandlerClient.setServer(IPAddress(services->getServiceIP("mqtt")), int(services->getServicePort("mqtt")));
                Serial.println("Server successfully set!");
                this->mqttHandlerClient.setClient(wifiManager->getRefWiFiClient());
                this->mqttHandlerClient.connect("ESP");
                Serial.println("MQTT connect success");
                setServerSuccess = true;
                delay(10000);
                return true;
            }
            else
            {
                if(FM->fExist(services->getExternalServiceFilename("mqtt", true).c_str()))
                {
                    logging.SFLog(className, "connect - AUTO", "Choose Fallback MQTT Broker", 0);
                    mqttHandlerClient.setServer(services->getServiceIP("mqtt", true), services->getServicePort("mqtt", true));
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
    return false;
}

bool MQTTHandler::connect(const char* id)
{
    logging.SFLog(className, "connect(id)", "try to connect with id only");

    return mqttHandlerClient.connect(id);
}

bool MQTTHandler::connect(const char* id, const char* user, const char* pass)
{
    logging.SFLog(className, "connect(id, user, pass)", "try to connect with id, user, pass");

    return mqttHandlerClient.connect(id, user, pass);
}

bool MQTTHandler::connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage)
{
    logging.SFLog(className, "connect(id, user, pass, will*)", "try to connect");

    return mqttHandlerClient.connect(id, willTopic, willQos, willRetain, willMessage);
}

bool MQTTHandler::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage)
{
    logging.SFLog(className, "connect(id, user, pass, will*)", "try to connect");

    return mqttHandlerClient.connect(id, user, pass, willTopic, willQos, willRetain, willMessage);
}

bool MQTTHandler::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession)
{
    logging.SFLog(className, "connect full", "try to connect");

    return mqttHandlerClient.connect(id, user, pass, willTopic, willQos, willRetain, willMessage, cleanSession);
}

/*
    Disconnect
*/

void MQTTHandler::disconnect()
{
    logging.SFLog(className, "disconnect", "Disconnecting from MQTT Broker!");
    mqttHandlerClient.disconnect();
}


/*
    Publishing
*/

bool MQTTHandler::publish(const char* topic, const char* payload)
{
    logging.SFLog(className, "publish", "Publish Message");

    return mqttHandlerClient.publish(topic, payload);
}

/*
    Subscription Control
*/
bool MQTTHandler::subscribe(const char* topic)
{
    logging.SFLog(className, "subscribe", "Subscribe topic");
    return mqttHandlerClient.subscribe(topic);
}

bool MQTTHandler::unsubscribe(const char* topic)
{
    logging.SFLog(className, "unsubscribe", "Unsubscribe topic");
    return mqttHandlerClient.unsubscribe(topic);
}


void MQTTHandler::eventListener(char* topic, byte* payload, uint length)
{
    lastCallback.topic = topic;

    String res = "";
    for(int i = 0; i < length; i++)
    {
        res += payload[i];
        Serial.println(payload[i]);
    }
    Serial.print("res: ");
    Serial.println(res);
    lastCallback.payload = res.c_str();
    res.~String();

    Serial.println("Settet: ");
    Serial.println(lastCallback.topic);
    Serial.println(lastCallback.payload);
}

lastMqttCallback* MQTTHandler::getCallback()
{
    return &this->lastCallback;
}

/*
    Other
*/

bool MQTTHandler::isConnected()
{
   return mqttHandlerClient.connected();
}

void MQTTHandler::init()
{
    mqttHandlerClient.setCallback(([this] (char* topic, byte * payload, unsigned int length) {this -> eventListener(topic, payload, length);}));
}

void MQTTHandler::run()
{
    
    if(!mqttHandlerClient.loop())
    {
        this->error.error = true;
        this->error.ErrorCode = 365;
        this->error.message = "Error wile running!";
        this->error.priority = 5;
    }

    delay(10); //need to fix several stability problems look at https://github.com/256dpi/arduino-mqtt?utm_source=platformio&utm_medium=piohome#notes
    
}

/*
    Inherited overwritten functionalities
*/
void MQTTHandler::startClass()
{
    if(this->classDisabled)
    {
        this->classDisabled = false;
        if(this->configCheck())
        {
            this->connect();
        }
    }
    else
    {
        this->connect();
    }
}

void MQTTHandler::stopClass()
{
    if(!this->classDisabled)
    {
        this->classDisabled = true;
        this->run();
        Serial.println("locked!");
        return;
    }
}

void MQTTHandler::pauseClass()
{
    this->stopClass();
}

void MQTTHandler::restartClass()
{
    this->startClass();
}

void MQTTHandler::continueClass()
{
    this->startClass();
}