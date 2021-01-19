#include "mqttHandler.h"

//extra Stuff
bool MQTTHandler::configCheck()
{
    if(FM->fExist(configFile))
    {
        return true;
        #ifdef J54J6_SysLogger
            
            logging.logIt("configCheck", "configFile already exist - SKIP!");
        #endif
    }
    else
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
    //if service will already searched then Skip other calls
    if(this->serviceAddDelayActive)
    {
        return false;
    }
    //end service will already searched in Network

    //start connecting to MQTT Broker
    logging.logIt("connect - AUTO", "Try to connect to MQTT Broker - read Config", 0);


    //Check for defined MQTT Broker Service - Extrnal and internal
    if(services->checkForService("mqtt", false) == 0 || searchService)
    {
        /*
            No Service defined on the device - try to search in Network for Devices with "NetworkIdent" - by j54j6
        */
        if(!searchService)
        {
            logging.logIt("connect - AUTO", "No MQTT Service defined - try to find Service in Network by Network", 1);
        }
        else
        {
            logging.logIt("connect - AUTO", "MQTT Search Service was poked by Connect - remove Backup CFG - Auto - start Searching for Service", 1);
            if(services->checkForService("mqtt", false) == 3 || services->checkForService("mqtt", false) == 5)
            {
                if(services->delService("mqtt", false, true))
                {
                    logging.logIt("connect - AUTO", "Successfully removed Backup CFG");
                }
                else
                {
                    logging.logIt("connect - AUTO", "Error while removing Backup CFG!", 2);
                } 
            }
        }
        
        /*
        if(mqttHandlerClient.connect(String(wifiManager->getDeviceMac()).c_str())) //check if any other connection is saved in Object e.g via setServer()
        {
            logging.logIt("connect - AUTO", "Successfully connected to in Object defined MQTT Server - return true", 0);
            return true;
        }
        else
        {
            logging.logIt("connect - AUTO", "Can't connect to MQTT Broker - Maybe no Address set?", 0);
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
               logging.logIt("connectAuto - Search", "Timeout reached - no Device Found");
               serviceAddDelayActive = false;
               break;
           }
           if(deviceFound == 3) 
           {
               logging.logIt("connectAuto - Search", "Service already defined! - SKIP");
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
            logging.logIt("connect - SearchServiceEnd", message.c_str());
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
       bool connectSuccess = false;
       const char* username = NULL;
       const char* passwd = NULL;
       const char* willTopic = 0;
       const char* willMessage = 0;
       uint8_t willQos = -1;
       bool willRetain = false;
       bool cleanSession = false;
       
       //Set WifiCLient as MQTTClient
       this->mqttHandlerClient.setClient(wifiManager->getRefWiFiClient());

       //Set all Parameters from Config File (defined above) by Config File
       if(FM->fExist(configFile))
       {
           logging.logIt("connect C1", "MQTT Config exist - read needed parameter");   
                            
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
        uint sPort = 0;
       for(int serviceTry = 0; serviceTry <= 3; serviceTry++)
       {
           if(connectSuccess)
           {
               return true;
           }
           switch(serviceTry)
           {
                case 1: //Try to connect to main CFG Stuff or internal
                    logging.logIt("connect C1", "Jump In - Servicetry Case 1", -1);        
                    switch(services->checkForService("mqtt", onlyUseExternal))
                    {
                        case 0:
                            logging.logIt("connect C1", "Can't find any defined Service - Try to find MQTT Broker in Network");   
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
                                logging.logIt("connect C1", "Successfully connected to MQTT Broker (localhost)");
                                connectSuccess = true;
                            }
                            else
                            {
                                logging.logIt("connect C1", "Can't connect to MQTT Broker! - only self Offered found and not connectable!");
                                connectSuccess = false;
                            }
                            break; //End Case 1 - only self offered Service found

                        default: //try to connect to Main CFG if defined
                            logging.logIt("connect C2", "Try to connect to Main CFG broker");
                            this->mqttHandlerClient = this->mqttHandlerClient.setServer(IPAddress(services->getServiceIP("mqtt")), int(services->getServicePort("mqtt")));
                            if(this->mqttHandlerClient.connect(WiFi.macAddress().c_str(), username, passwd, willTopic, willQos, willRetain, willMessage, cleanSession))
                            {
                                logging.logIt("connect C1", "Successfully connected to MQTT Broker (Main CFG)");
                                connectSuccess = true;
                            }
                            else
                            {
                                logging.logIt("connect C1", "Can't connect to MQTT Broker! - Main CFG not connectable!");
                                connectSuccess = false;
                            }
                            break; //End Case2 - Connect to Main CFG
                    };
                break; //End Case 1 - try to connect to Main Services

                case 2: //Try to connect to Fallback Services
                    logging.logIt("connect S2", "Jump In - Service try Case 2", -1);    
                    switch(services->checkForService("mqtt", onlyUseExternal))
                    {
                        case 0:
                        case 2:
                            logging.logIt("connect S2", "Can't find any defined Service / Fallback Service - Skip S2 - Try to find MQTT Broker in Network");   
                            break; //End Case 0/2

                        case 1: //Second try to connect to MQTT Service with default port 1883 tcp
                            this->mqttHandlerClient.setServer("127.0.0.1", 1883);
                            if(this->mqttHandlerClient.connect(WiFi.macAddress().c_str(), username, passwd, willTopic, willQos, willRetain, willMessage, cleanSession))
                            {
                                logging.logIt("connect S2", "Successfully connected to MQTT Broker (localhost)");
                                connectSuccess = true;
                            }
                            else
                            {
                                logging.logIt("connect S2", "Can't connect to MQTT Broker! - only self Offered found and not connectable!");
                                connectSuccess = false;
                            }
                            break; //End Case 1 - only self offered Service found

                        case 3: //try to connect to Backup CFG if defined
                        case 5: 
                            logging.logIt("Connect S2-C3/5", "Try to connect to Backup CFG broker");
                            this->mqttHandlerClient.setServer(this->services->getServiceIP("mqtt", true), this->services->getServicePort("mqtt", true));

                            if(this->mqttHandlerClient.connect(WiFi.macAddress().c_str(), username, passwd, willTopic, willQos, willRetain, willMessage, cleanSession))
                            {
                                logging.logIt("Connect S2-C3/5", "Successfully connected to MQTT Broker (Backup CFG)");
                                connectSuccess = true;
                            }
                            else
                            {
                                logging.logIt("Connect S2-C3/5", "Can't connect to MQTT Broker! - Backup CFG not connectable!");
                                connectSuccess = false;
                            }
                            break; //End Case3/5
                    };
                    break; //End Case2 - Connect to Backup CFG

                case 3: //Try to Search another device in Network
                    logging.logIt("connect C3", "Jump In - Servicetry Case 3", -1);   
                    for(int tries = 0; tries <= 2; tries++)
                    {
                        if(this->connect(true, true))
                        {
                            if(tries <= 1)
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
            }//End switch ServiceTry
        } //End For loop
        logging.logIt("connect - Main", "Can't connect to broker - connect Failed!", 1);
        return false;
    } //End Else - try to connect to defined service
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

    if(serviceAddDelayActive)
    {
        if(serviceAddDelayTimeout <= millis())
        {
            serviceAddDelayTimeout = 0;
            serviceAddDelayActive = false;
            logging.logIt("run - ServiceAdd", "ServiceAddDelayTimeout reached - try to connect to mqtt Service");
            this->connect();
        }
    }
    
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
        Serial.println("locked!");
        return;
    }
}

void MQTTHandler::restartClass()
{
    this->startClass();
}