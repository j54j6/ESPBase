#include "mqttHandler.h"

//extra Stuff
bool MQTTHandler::configCheck()
{
    bool needReInit = false;
    if(FM->fExist(configFile))
    {
        #ifdef J54J6_SysLogger
            
            logging.logIt("configCheck", "configFile already exist - check for Parameter!");
        #endif

        if(!FM->checkForKeyInJSONFile(configFile, "updateServer") || 
        !FM->checkForKeyInJSONFile(configFile, "uri") ||
        !FM->checkForKeyInJSONFile(configFile, "servertoken") ||
        !FM->checkForKeyInJSONFile(configFile, "serverpass") ||
        !FM->checkForKeyInJSONFile(configFile, "port") ||
        !FM->checkForKeyInJSONFile(configFile, "updateSearch") ||
        !FM->checkForKeyInJSONFile(configFile, "autoUpdate") ||
        !FM->checkForKeyInJSONFile(configFile, "softwareVersion") ||
        !FM->checkForKeyInJSONFile(configFile, "checkDelay"))
        {
            needReInit = true;
            #ifdef J54J6_SysLogger
                logging.logIt("configCheck", "Config File exist but file is not valid - one or more keys are missing");
            #endif
        }
        else
        {
            return true;
        }
        
    }

    if(!FM->fExist(configFile) || needReInit)
    {
        #ifdef J54J6_SysLogger
            
            logging.logIt("configCheck", "configFile don't exist - try to create");
        #endif

        if(!FM->createFile(configFile))
        {
            #ifdef J54J6_SysLogger
            
            logging.logIt("configCheck", "Can't create File", 2);
            #endif
            classControl.newReport("Can't create Config File!", 321, 5, true);
            return false;
        }
        
        if(!FM->writeJsonFile(configFile, configFallback, 0))
        {
            #ifdef J54J6_SysLogger
            
            logging.logIt("configCheck", "Can't write in File", 2);
            #endif
            classControl.newReport("Can't write in File", 322, 5, true);
            return false;
        }
        #ifdef J54J6_SysLogger
            logging.logIt("configCheck", "Config check successfull");
        #endif
        return true;
    }
    return false;
}

void MQTTHandler::lockConnect(String usedIP, bool isError)
{
    lastUsedIP = usedIP;
    if(isError) //Can't connect to Broker
    {
        logging.logIt("lockConnect", "locked Connect cause of Error!", 1);
        if(connectTries >= tryToConnectXTimes)
        {
            connectLocked = true;
            unlockAt = millis() + timeoutAfterSearchError;
            lockReasonIsError = true;
        }
        else
        {
            connectTries++;
        }
    }
    else //ServiceAdd Delay enabled
    {
        logging.logIt("lockConnect", "locked Connect cause of ServiceAdd Delay!", 1);
        connectLocked = true;
        unlockAt = millis() + serviceSearchTimeout;
    }
}

void MQTTHandler::checklockConnectTimer()
{
    if(unlockAt != 0)
    {
        if(unlockAt <= millis())
        {
            resetConnectLock();
        }
    }
}

void MQTTHandler::resetConnectLock()
{
    unlockAt = 0;
    connectLocked = false;
    if(lockReasonIsError && connectTries >= tryToConnectXTimes)
    {
        connectTries = 0;
    }
    else if(!lockReasonIsError)
    {
        connectOrError = true;
    }
}

short MQTTHandler::connectSelfOffered()
{
    logging.logIt("connectSelfOffered", "Try to connect to self Offered Broker", 3);
    int port = -1;
    port = services->getServicePort("mqtt", false, true);
    if(port == -1 || port < 1 || port > 65535)
    {
        logging.logIt("connectSelfOffered", "Can't connect to self offered Broker - config not useable!", 3);
        return 2; //no useable 
    }
    else
    {
        this->mqttHandlerClient = this->mqttHandlerClient.setServer("127.0.0.1", port);
        if(this->mqttHandlerClient.connect(wifiManager->getDeviceMac().c_str()))
        {
            logging.logIt("connectSelfOffered", "Successfully connected to self offered Broker!", 1);
            return true;
        }

        logging.logIt("connectSelfOffered", "Can't connect to self offered Broker!", 1);
        return false;
    }
    logging.logIt("connectSelfOffered", "Unexcepted behaviour!", 4);
    return -1;
}

short MQTTHandler::connectToExternalBroker(bool isFallback)
{
    short serviceConnectMode = services->checkForService("mqtt");

    if((serviceConnectMode == 0 || serviceConnectMode == 1) || (isFallback && (serviceConnectMode == 2 || serviceConnectMode == 4)))
    {
        logging.logIt("connectToExternalBroker", "No useable Service found", 2);
        return 2; //There is no correct Service found (if isFallback) -> there is no fallback
    }

    if(!isFallback && (serviceConnectMode == 2 || serviceConnectMode == 3 || serviceConnectMode == 4 || serviceConnectMode == 5)) //connect to mainCFG
    {
        logging.logIt("connectToExternalBroker", "Use external Main Config", 2);
        IPAddress sIP = services->getServiceIP("mqtt");
        int port = services->getServicePort("mqtt", false, false);
        this->mqttHandlerClient = this->mqttHandlerClient.setServer(sIP, port);
        return this->mqttHandlerClient.connect(wifiManager->getDeviceMac().c_str());  
    }
    else if(isFallback && (serviceConnectMode == 3 || serviceConnectMode == 5))
    {
        logging.logIt("connectToExternalBroker", "Use external Fallback Config", 2);
        IPAddress sIP = services->getServiceIP("mqtt", true);
        int port = services->getServicePort("mqtt", true, false);
        this->mqttHandlerClient = this->mqttHandlerClient.setServer(sIP, port);
        return this->mqttHandlerClient.connect(wifiManager->getDeviceMac().c_str()); 
    }
    else
    {
        logging.logIt("connectToExternalBroker", "No useable Service(2) found", 2);
        return 2;
    }
    
}

short MQTTHandler::connectToMQTTBroker()
{
    logging.logIt("connectToMQTTBroker", "Try to connect to Conf Broker", 2);
    logging.logIt("connectToMQTTBroker", "checkForMQTT Service!", 1);
    short connectMode = services->checkForService("mqtt");

    switch(connectMode)
    {
        case 1: //only self offered - search if failing - ip = localhost
            if(connectSelfOffered() == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to internal MQTT broker", 2);
                return 1;
            }
            else
            {
                logging.logIt("connectToMQTTBroker", "Can't connect to internal Broker - no other conf - start autoAdd", 2);
                services->autoAddService("mqtt");
                lockConnect("localhost"); //start auto add and lock connect to prevent new init
                return 4;
            }
            break;

        case 2: //only 1 external Config found - no Internal -> search if failing
            if(connectToExternalBroker() == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to Main External MQTT broker", 2);
                return 1;
            }
            else
            {
                logging.logIt("connectToMQTTBroker", "Can't connect to Main CFG Broker - no other conf - start autoAdd", 2);
                services->autoAddService("mqtt");
                lockConnect("mainConfig"); //start auto add and lock connect to prevent new init
                return 4;
            }
            break;

        case 3: // 2 external found - check both - if error search in newtwork and remove backup
            if(connectToExternalBroker() == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to Main External MQTT broker", 2);
                return 1;
            }
            else if(connectToExternalBroker(true) == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to backup External MQTT broker", 2);
                return 1;
            }
            else
            {
                logging.logIt("connectToMQTTBroker", "Can't connect to Main/Backup CFG Broker - no other conf - start autoAdd", 2);
                services->autoAddService("mqtt");
                lockConnect("FallbackConfig"); //start auto add and lock connect to prevent new init
                return 4;
            }
            break;
        case 4: //1 external and self offered found - try selfoffered first then external -> if fail -> search
            if(connectSelfOffered() == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to internal MQTT broker", 2);
                return 1;
            }
            else if(connectToExternalBroker() == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to Main External MQTT broker", 2);
                return 1;
            }
            else
            {
                logging.logIt("connectToMQTTBroker", "Can't connect to Internal or Main CFG Broker - no other conf - start autoAdd", 2);
                services->autoAddService("mqtt");
                lockConnect("FallbackConfig"); //start auto add and lock connect to prevent new init
                return 4;
            }
            break;
        case 5: // 2 external and self offered found - try selfofferd first then both external -> if fail remove backup and search
            if(connectSelfOffered() == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to internal MQTT broker", 2);
                return 1;
            }
            if(connectToExternalBroker() == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to Main External MQTT broker", 2);
                return 1;
            }
            else if(connectToExternalBroker(true) == 1)
            {
                logging.logIt("connectToMQTTBroker", "Successfully connected to backup External MQTT broker", 2);
                return 1;
            }
            else
            {
                logging.logIt("connectToMQTTBroker", "Can't connect to Internal or Main/Backup CFG Broker - no other conf - start autoAdd", 2);
                services->autoAddService("mqtt");
                lockConnect("FallbackConfig"); //start auto add and lock connect to prevent new init
                return 4;
            }
            break;
    };
    return false;
}

//Constructor

MQTTHandler::MQTTHandler(Filemanager* FM, WiFiManager* wifiManager, ServiceHandler* serviceHandler)
{
    this->FM = FM;
    this->wifiManager = wifiManager;
    this->services = serviceHandler;
    this->logging = SysLogger(FM, "MQTTHandler");
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
            message += "IP: ";
            message += ipChanged;
            message += "\n Port: ";
            message += portChanged;
            logging.logIt("setServer", message.c_str(), 2);

            classControl.newReport(message, 232, 2, false, true);
            return false;
        }
    }
    else if (((save & asFallback) && services->checkForService(services->getExternalServiceFilename("mqtt", true).c_str()) == 0) || services->checkForService(services->getExternalServiceFilename("mqtt", true).c_str()) == 2)
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
            logging.logIt("setServer - FallbackPart", message.c_str(), 2);

            classControl.newReport(message, 232, 2, false, true);
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
        logging.logIt("setID", "ID can't be NULL - return false", 1);
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
        logging.logIt("setUser", "user can't be NULL - return false", 1);
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
        logging.logIt("setPass", "Pass can't be NULL - return false", 1);
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
        logging.logIt("willTopic", "willTopic can't be NULL - return false", 1);
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
        logging.logIt("setWillMessage", "WillMessage can't be NULL - return false", 1);
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
    Get STuff
*/

String MQTTHandler::getWillMessage()
{
    if(!FM->fExist(configFile))
    {
        return "0";
    }

    return FM->readJsonFileValue(configFile, "willMessage");
}

String MQTTHandler::getWillTopic()
{
    if(!FM->fExist(configFile))
    {
        return "0";
    }

    return FM->readJsonFileValue(configFile, "willTopic");
}

uint8_t MQTTHandler::getWillQos()
{
    if(!FM->fExist(configFile))
    {
        return 0;
    }
    String cache = FM->readJsonFileValue(configFile, "willQos");
    if(cache.toInt())
    {
        return cache.toInt();
    }
    return 0;
}

bool MQTTHandler::getWillRetain()
{
    if(!FM->fExist(configFile))
    {
        return false;
    }

    return FM->returnAsBool(FM->readJsonFileValue(configFile, "willRetain"));
}

bool MQTTHandler::getCleanSession()
{
    if(!FM->fExist(configFile))
    {
        return true;
    }

    return FM->returnAsBool(FM->readJsonFileValue(configFile, "cleanSession"));
}

/*
    Connect
*/

bool MQTTHandler::connect(bool onlyUseExternal, bool searchService)
{
    logging.logIt("connect(auto)", "Init auto Connect", 2);

    //If connect was already called "serviceAddDelayActive" is enabled to prevent multiple requests - return until the timer is ended
    if(connectLocked) //the connectFunction is locked because of errors or serviceAdd is Active (second Value)
    {
        return false;
    }

    //check for defined Service
    logging.logIt("connect(auto)", "Check for defined mqtt Service", 1);
    short serviceDefined = services->checkForService("mqtt");

    if(serviceDefined != 0) //connect to mqtt Service defined
    {
        logging.logIt("connect(auto)", "Service found! - try to connect - call conecctToMQTTBroker()", 1);
        short res = connectToMQTTBroker();
        if(res == 1)
        {
            logging.logIt("connect(auto)", "Successfully connected to MQTT", 2);
            return true;
        }
        else
        {
            logging.logIt("connect(auto)", "Can't connect to MQTT Broker", 1);
            return false;
        }
    }
    else if(serviceDefined == 0 && connectOrError) //if connectOrError is active - autoAddDelayTimer is over - if there is no Service it is failed!s
    {
        logging.logIt("connect(auto)", "After ServiceAdd can't find MQTT Service!");
        connectTries++;
    }
    else
    {
        services->autoAddService("mqtt");
        return false;
    }

return false;
} //End connect()

bool MQTTHandler::connect(const char* id)
{
    logging.logIt("connect(id)", "try to connect with id only");

    if(strcmp(id, "") == 0)
    {
        return mqttHandlerClient.connect(String(wifiManager->getDeviceMac()).c_str());
    }
    return mqttHandlerClient.connect(id);    
}

bool MQTTHandler::connect(const char* id, const char* user, const char* pass)
{
    logging.logIt("connect(id, user, pass)", "try to connect with id, user, pass");

    return mqttHandlerClient.connect(id, user, pass);
}

bool MQTTHandler::connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage)
{
    logging.logIt("connect(id, user, pass, will*)", "try to connect");

    return mqttHandlerClient.connect(id, willTopic, willQos, willRetain, willMessage);
}

bool MQTTHandler::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage)
{
    logging.logIt("connect(id, user, pass, will*)", "try to connect");

    return mqttHandlerClient.connect(id, user, pass, willTopic, willQos, willRetain, willMessage);
}

bool MQTTHandler::connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession)
{
    logging.logIt("connect full", "try to connect");

    return mqttHandlerClient.connect(id, user, pass, willTopic, willQos, willRetain, willMessage, cleanSession);
}

/*
    Connect Helper
*/

/*
    Disconnect
*/

void MQTTHandler::disconnect()
{
    logging.logIt("disconnect", "Disconnecting from MQTT Broker!");
    mqttHandlerClient.disconnect();
}


/*
    Publishing
*/

bool MQTTHandler::publish(const char* topic, const char* payload)
{
    logging.logIt("publish", "Publish Message");

    return mqttHandlerClient.publish(topic, payload);
}

/*
    Subscription Control
*/
bool MQTTHandler::subscribe(const char* topic)
{
    logging.logIt("subscribe", "Subscribe topic");
    return mqttHandlerClient.subscribe(topic);
}

bool MQTTHandler::unsubscribe(const char* topic)
{
    logging.logIt("unsubscribe", "Unsubscribe topic");
    return mqttHandlerClient.unsubscribe(topic);
}


void MQTTHandler::eventListener(char* topic, uint8* payload, uint length)
{
    /*
    lastCallback.topic = topic;

    String res = "";
    for(uint i = 0; i < length; i++)
    {
        res += (char)payload[i];
    }
    lastCallback.payload = res.c_str();


    Serial.println("Settet: ");
    Serial.println(lastCallback.topic);
    Serial.println(lastCallback.payload);
*/
    String res;
    lastCallback.topic = topic;

  for (uint i = 0; i < length; i++) {
    res += (char)payload[i];
  }
  lastCallback.payload = res;
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
    mqttHandlerClient.setCallback(([this] (char* topic, uint8* payload, uint length) {MQTTHandler::eventListener(topic, payload, length);}));
    this->mqttHandlerClient.setClient(wifiManager->getRefWiFiClient());
}

void MQTTHandler::run()
{
    lastCallback.reset();
    checklockConnectTimer();
    if(this->wifiManager->isConnected() && this->isConnected())
    {
        if(!mqttHandlerClient.loop())
        {
            classControl.newReport("Error while running - mqttHandler return false!", 365, 5, true, true);
        }
    }
    classControl.run();
    //delay(10); //need to fix several stability problems look at https://github.com/256dpi/arduino-mqtt?utm_source=platformio&utm_medium=piohome#notes
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
        return;
    }
}

void MQTTHandler::restartClass()
{
    this->startClass();
}