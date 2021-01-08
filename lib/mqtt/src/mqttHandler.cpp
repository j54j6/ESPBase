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
    else if ((save & asFallback) && services->checkForService(services->getExternalServiceFilename("mqtt", true).c_str()) == 0 || services->checkForService(services->getExternalServiceFilename("mqtt", true).c_str()) == 2)
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
    Get STuff
*/

String MQTTHandler::getWillMessage()
{
    if(!FM->fExist(configFile))
    {
        return {};
    }

    return FM->readJsonFileValue(configFile, "willMessage");
}

String MQTTHandler::getWillTopic()
{
    if(!FM->fExist(configFile))
    {
        return {};
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
        return false;
    }

    return FM->returnAsBool(FM->readJsonFileValue(configFile, "cleanSession"));
}

/*
    Connect
*/

bool MQTTHandler::connect(bool onlyUseExternal, bool searchService)
{
    //if service will already searched then Skip other calls
    if(this->serviceAddDelayActive)
    {
        return false;
    }
    //end service will already searched in Network

    //start connecting to MQTT Broker
    logging.SFLog(className, "connect - AUTO", "Try to connect to MQTT Broker - read Config", 0);


    //Check for defined MQTT Broker Service - Extrnal and internal
    if(!services->checkForService("mqtt", false) || searchService)
    {
        /*
            No Service defined on the device - try to search in Network for Devices with "NetworkIdent" - by j54j6
        */
        if(!searchService)
        {
            logging.SFLog(className, "connect - AUTO", "No MQTT Service defined - try to find Service in Network by Network", 1);
        }
        else
        {
            logging.SFLog(className, "connect - AUTO", "MQTT Search Service was poked by Connect - remove Backup CFG - Auto - start Searching for Service", 1);
            if(services->delService("mqtt", false, true))
            {
                logging.SFLog(className, "connect - AUTO", "Successfully removed Backup CFG");
            }
            else
            {
                logging.SFLog(className, "connect - AUTO", "Error while removing Backup CFG!", 2);
            } 
        }
        
        /*
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
        */
       this->serviceAddDelayActive = true; //set function in "InSearch" Mode to prevent multipe search requests in single-System
       this->serviceAddDelayTimeout = millis() + 5000; // Set Search Timeout - After Timeout Search Request will canceled
       short deviceFound = 100; //init value for "deviceFound" - 100 is a random number and has no special meaning

       while(deviceFound != 1 && serviceAddDelayActive) //while no device is found 
       {
           deviceFound = services->autoAddService("mqtt"); //NetworkIdent -> searchDevice in Network with defined "mqtt" device
           if(millis() > serviceAddDelayTimeout) //if the timeour is reached stop searching
           {
               logging.SFLog(className, "connectAuto - Search", "Timeout reached - no Device Found");
               serviceAddDelayActive = false;
               break;
           }
           if(deviceFound == 3) 
           {
               logging.SFLog(className, "connectAuto - Search", "Service already defined! - SKIP");
               serviceAddDelayActive = false;
               break;
           }
           services->loop();
       }
        
        if(serviceAddDelayActive == false && deviceFound != 100)
        {
            String message;
            message = "Network Search timeout reached - No Device Found\nLast State:";
            message += deviceFound;
            logging.SFLog(className, "connect - SearchServiceEnd", message.c_str());
            deviceFound = 100;
            return false;
        }       
        else
        {
            return true;
        }
        
    }
    else //MQTT Service defined
    {
        /*
        bool setServerSuccess = false;
        for(int i = 0; i <= 1; i++)
        {
            if(i == 0)
            {   
                logging.SFLog(className, "connect - AUTO", "Choose Main MQTT Broker", 0);
                this->mqttHandlerClient = this->mqttHandlerClient.setServer(IPAddress(services->getServiceIP("mqtt")), int(services->getServicePort("mqtt")));
                Serial.println("Server successfully set!");
                this->mqttHandlerClient.setClient(wifiManager->getRefWiFiClient());
                this->mqttHandlerClient.connect(String(wifiManager->getDeviceMac()).c_str());
                Serial.println("MQTT connect success");
                setServerSuccess = true;
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
        */

       const char* username = "";
       const char* passwd = "";
       const char* willTopic = "";
       const char* willMessage = "";
       uint8_t willQos = -1;
       bool willRetain = false;
       bool cleanSession = false;
    
       //Set all Parameters from Config File (defined above) by Config File
       if(FM->fExist(configFile))
       {
           const char* valUsername = FM->readJsonFileValue(configFile, "user");

           if(!strcmp(valUsername, "") == 0 || !valUsername)
           {
               username = valUsername;
           }

            const char* valPassword = FM->readJsonFileValue(configFile, "pass");

           if(!strcmp(valPassword, "") == 0 || !valPassword)
           {
               username = valPassword;
           }

           willTopic = this->getWillTopic().c_str();
           willQos = this->getWillQos();
           willMessage = this->getWillMessage().c_str();
           willRetain = this->getWillRetain();
           cleanSession = this->getCleanSession();
       }
        bool connectSuccess = false;
        uint sPort = 0;
       for(int serviceTry = 0; serviceTry <= 3; serviceTry++)
       {
           switch(serviceTry)
           {
                case 1: //Try to connect to main CFG Stuff or internal   
                    switch(services->checkForService("mqtt", onlyUseExternal))
                    {
                        case 0:
                            logging.SFLog(className, "connect C1", "Can't find any defined Service - Try to find MQTT Broker in Network");   
                            break; //End Case 0

                        case 1:
                            sPort = this->services->getServicePort("mqtt", false, true);

                            if(sPort < 1 || sPort > 65535 || !sPort)
                            {
                                sPort = 1883; //default port
                            }
                            this->mqttHandlerClient.setServer("127.0.0.1", sPort);
                            if(this->mqttHandlerClient.connect(WiFi.macAddress().c_str(), username, passwd, willTopic, willQos, willRetain, willMessage, cleanSession))
                            {
                                logging.SFLog(className, "connect C1", "Successfully connected to MQTT Broker (localhost)");
                                connectSuccess = true;
                            }
                            else
                            {
                                logging.SFLog(className, "connect C1", "Can't connect to MQTT Broker! - only self Offered found and not connectable!");
                                connectSuccess = false;
                            }
                            break; //End Case 1 - only self offered Service found

                        default: //try to connect to Main CFG if defined
                            logging.SFLog(className, "connect C2", "Try to connect to Main CFG broker");
                            this->mqttHandlerClient.setServer(this->services->getServiceIP("mqtt"), this->services->getServicePort("mqtt"));

                            if(this->mqttHandlerClient.connect(WiFi.macAddress().c_str(), username, passwd, willTopic, willQos, willRetain, willMessage, cleanSession))
                            {
                                logging.SFLog(className, "connect C1", "Successfully connected to MQTT Broker (Main CFG)");
                                connectSuccess = true;
                            }
                            else
                            {
                                logging.SFLog(className, "connect C1", "Can't connect to MQTT Broker! - Main CFG not connectable!");
                                connectSuccess = false;
                            }
                        break; //End Case2 - Connect to Main CFG
                    };
                break; //End Case 1 - try to connect to Main Services

                case 2: //Try to connect to Fallback Services
                    switch(services->checkForService("mqtt", onlyUseExternal))
                    {
                        case 0:
                            logging.SFLog(className, "connect C1", "Can't find any defined Service - Try to find MQTT Broker in Network");   
                            break; //End Case 0

                        case 1: //Second try to connect to MQTT Service with default port 1883 tcp
                            this->mqttHandlerClient.setServer("127.0.0.1", 1883);
                            if(this->mqttHandlerClient.connect(WiFi.macAddress().c_str(), username, passwd, willTopic, willQos, willRetain, willMessage, cleanSession))
                            {
                                logging.SFLog(className, "connect C1", "Successfully connected to MQTT Broker (localhost)");
                                connectSuccess = true;
                            }
                            else
                            {
                                logging.SFLog(className, "connect C1", "Can't connect to MQTT Broker! - only self Offered found and not connectable!");
                                connectSuccess = false;
                            }
                            break; //End Case 1 - only self offered Service found

                        case 3: //try to connect to Backup CFG if defined
                        case 5: 
                            logging.SFLog(className, "connect S2-C3/5", "Try to connect to Backup CFG broker");
                            this->mqttHandlerClient.setServer(this->services->getServiceIP("mqtt", true), this->services->getServicePort("mqtt", true));

                            if(this->mqttHandlerClient.connect(WiFi.macAddress().c_str(), username, passwd, willTopic, willQos, willRetain, willMessage, cleanSession))
                            {
                                logging.SFLog(className, "connect C1", "Successfully connected to MQTT Broker (Backup CFG)");
                                connectSuccess = true;
                            }
                            else
                            {
                                logging.SFLog(className, "connect C1", "Can't connect to MQTT Broker! - Backup CFG not connectable!");
                                connectSuccess = false;
                            }
                            break; //End Case3/5
                    };
                    break; //End Case2 - Connect to Backup CFG

                case 3: //Try to Search another device in Network
                    default:
                        for(int tries = 0; tries <= 3; tries++)
                        {
                            if(this->connect(true, true))
                            {
                                if(tries < 3)
                                {
                                    connectSuccess = connect();
                                }
                                else
                                {
                                    connectSuccess = false;
                                    break;
                                }
                            }              
                        }
                break; //End Case 3 - Search for new Service in Network

                if(connectSuccess)
                {
                    return true;
                }
            }//End switch ServiceTry
        } //End For loop
        return false;
    } //End Else - try to connect to defined service
} //End connect()

bool MQTTHandler::connect(const char* id)
{
    logging.SFLog(className, "connect(id)", "try to connect with id only");

    if(strcmp(id, "") == 0)
    {
        return mqttHandlerClient.connect(String(wifiManager->getDeviceMac()).c_str());
    }
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
    Connect Helper
*/

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
}

void MQTTHandler::run()
{

    if(serviceAddDelayActive)
    {
        if(serviceAddDelayTimeout <= millis())
        {
            serviceAddDelayTimeout = 0;
            serviceAddDelayActive = false;
            logging.SFLog(className, "run - ServiceAdd", "ServiceAddDelayTimeout reached - try to connect to mqtt Service");
            this->connect();
        }
    }
    
    if(!mqttHandlerClient.loop())
    {
        this->error.error = true;
        this->error.ErrorCode = 365;
        this->error.message = "Error wile running!";
        this->error.priority = 5;
    }

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