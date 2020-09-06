#include "externServiceHandler.h"

ExternServiceHandler::ExternServiceHandler(Filemanager* FM, WiFiManager* wifiManager, NetworkIdent* networkIdent)
{
    this->FM = FM;
    this->wifiManager = wifiManager;
    this->networkIdent = networkIdent;

    //check for configFile
    if(!FM->fExist(serviceAddressListFile))
    {
        FM->fOpen(serviceAddressListFile, "w");
        FM->fClose();
        #ifdef J54J6_LOGGING_H //use logging Libary if included
            logger::SFLog(className, "Constructor", "Config File created");
        #endif
    }
    else
    {
        #ifdef J54J6_LOGGING_H //use logging Libary if included
            logger::SFLog(className, "Constructor", "ConfigFile already exist - Skip", 1);
        #endif
    }
}

int ExternServiceHandler::getServiceDestPort(const char* serviceName)
{
    if(FM->fExist(getFilename(serviceName).c_str()))
    {
        //service already added - readout File
        if(FM->checkForKeyInJSONFile(getFilename(serviceName).c_str(), "port"))
        {
            return int(FM->readJsonFileValue(getFilename(serviceName).c_str(), "port"));
        }        
    }
    else if(FM->fExist(getFilename(serviceName, true).c_str()))
    {
        if(FM->checkForKeyInJSONFile(getFilename(serviceName, true).c_str(), "port"))
        {
            return int(FM->readJsonFileValue(getFilename(serviceName).c_str(), "port"));
        }
    }
    
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "getServiceDestPort", "Can't find DestPort - search in network...", 0);
    #endif
    //if file not exist or key "port" is not defined in File search for service and recreate the File
    networkIdent->searchForService(serviceName);
    lastRequest.id = networkIdent->getLastGeneratedId();
    lastRequest.createdAt = millis();
    lastRequest.serviceName = serviceName;
    lastRequest.searchType = 2;
    return -1;
}

IPAddress ExternServiceHandler::getServiceIP(const char* serviceName)
{
    if(FM->fExist(getFilename(serviceName).c_str()))
    {
        //service already added - readout File
        if(FM->checkForKeyInJSONFile(getFilename(serviceName).c_str(), "ip"))
        {
            return int(FM->readJsonFileValue(getFilename(serviceName).c_str(), "ip"));
        }        
    }
    else if(FM->fExist(getFilename(serviceName, true).c_str()))
    {
        //service already added - readout File
        if(FM->checkForKeyInJSONFile(getFilename(serviceName, true).c_str(), "ip"))
        {
            return int(FM->readJsonFileValue(getFilename(serviceName, true).c_str(), "ip"));
        }   
    }
    

    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "getServiceIP", "Can't find ServiceIP - search in network...", 0);
    #endif

    //if file not exist or key "port" is not defined in File search for service and recreate the File
    networkIdent->searchForService(serviceName);
    lastRequest.id = networkIdent->getLastGeneratedId();
    lastRequest.createdAt = millis();
    lastRequest.serviceName = serviceName;
    lastRequest.searchType = 1;
    return IPAddress(0,0,0,0);
}

String ExternServiceHandler::getServiceMACAddress(const char* serviceName)
{
    if(FM->fExist(getFilename(serviceName).c_str()))
    {
        //service already added - readout File
        if(FM->checkForKeyInJSONFile(getFilename(serviceName).c_str(), "mac"))
        {
            return String(FM->readJsonFileValue(getFilename(serviceName).c_str(), "mac"));
        }        
    }
    else if(FM->fExist(getFilename(serviceName, true).c_str()))
    {
        //service already added - readout File
        if(FM->checkForKeyInJSONFile(getFilename(serviceName, true).c_str(), "mac"))
        {
            return String(FM->readJsonFileValue(getFilename(serviceName, true).c_str(), "mac"));
        }   
    }
    
    
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "getServiceIP", "Can't find ServiceMAC - search in network...", 0);
    #endif

    //if file not exist or key "port" is not defined in File search for service and recreate the File
    networkIdent->searchForService(serviceName);
    lastRequest.id = networkIdent->getLastGeneratedId();
    lastRequest.createdAt = millis();
    lastRequest.serviceName = serviceName;
    lastRequest.searchType = 3;
    return "false";
}


short ExternServiceHandler::autoAddService(const char* serviceName)
{
    if(!autoAddRunning && strcmp(serviceName, "n.S") != 0) //init. autoAdd and set all Variables
    {
        //check if service already is registered - main Cfg
        if(!FM->fExist(getFilename(serviceName).c_str()))
        {
            //init main CFG create
            #ifdef J54J6_LOGGING_H
                logger logging;
                String message = "Init. creation of mainCFG for Service \"";
                message += serviceName;
                message += "\"";
                logging.SFLog(className, "autoAddService", message.c_str(), 0);
            #endif

            networkIdent->searchForService(serviceName);
            lastRequest.createdAt = millis();
            lastRequest.deleteAfter = timeoutAfter;
            lastRequest.id = networkIdent->getLastGeneratedId(); 
            lastRequest.searchType = 4; //autoAdd function is used
            lastRequest.serviceName = serviceName;
            autoAddRunning = true; //start autoAdd loop part
            return 2;
        }
        else
        {
            if(!FM->fExist(getFilename(serviceName, true).c_str()))
            {
                #ifdef J54J6_LOGGING_H
                    logger logging;
                    String message = "Init. creation of FallbackCFG for Service \"";
                    message += serviceName;
                    message += "\"";
                    logging.SFLog(className, "autoAddService", message.c_str(), 0);
                #endif

                //init main CFG create
                networkIdent->searchForService(serviceName);
                lastRequest.createdAt = millis();
                lastRequest.deleteAfter = timeoutAfter;
                lastRequest.id = networkIdent->getLastGeneratedId(); 
                lastRequest.searchType = 4; //autoAdd function is used
                lastRequest.serviceName = serviceName;
                lastRequest.isFallback = true;
                autoAddRunning = true; //start autoAdd loop part
                return 2;
            }
            else
            {
                #ifdef J54J6_LOGGING_H
                    logger logging;
                    String message = "Can't create CFG for Service \"";
                    message += serviceName;
                    message += "\" - Fallback and Main already defined";
                    logging.SFLog(className, "autoAddService", message.c_str(), 0);
                #endif
                return 3;
            } 
        }        
    }
    else if(!autoAddRunning && strcmp(serviceName, "n.S") != 0) //serviceName is not "n.S"
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "autoAddService", "Can't add Service \"n.S\" - it's a placeholder!", 1);
        #endif
        return 0;
    }
    else //autoAdd already running - do loop stuff to check for any responses and add new service if needed
    {
        if(lastRequest.id == 0) //lastRequest is > timeout - disable autoRun
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "autoAddService", "Can't add Service - timeout reached! - return false", 1);
            #endif
            autoAddRunning = false;
            return 0;
        }
        else if(lastRequest.id != 0 && lastRequest.searchType != 4) //lasRequest was not set by function AutoAdd
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "autoAddService", "Schedule Problem - lastRequest not set by autoAdd - wrong searchType - disable autoAdd loop and return false", 1);
            #endif
            autoAddRunning = false;
            return 0;
        }
        
        //working normal
        if(lastRequest.id != 0 && lastRequest.searchType == 4)
        {
            StaticJsonDocument<425> lastFetched = networkIdent->getLastData();

            /*
                Check if lastFetched Data have the right Syntax and Values needed to add new Service
            */
            if(!lastFetched.containsKey("id") || !lastFetched.containsKey("serviceName") || !lastFetched.containsKey("ip") || !lastFetched.containsKey("mac") || !lastFetched.containsKey("servicePort"))
            {
                return 2;
            }
            else
            {
                //lastReceived Packet does contain all needed keys to create a new Service - check for id and serviceName
                String castedLastRequestID = String(lastRequest.id);
                String castedLastReceivedID = lastFetched["id"];

                castedLastReceivedID.replace(" ", "");
                castedLastRequestID.replace(" ", "");

                if(castedLastRequestID == castedLastReceivedID)
                {
                    //same id - start adding new service
                    if(!lastRequest.isFallback)
                    {
                        /*
                            Creating main Service Cfg
                        */
                        if(!FM->fExist(getFilename(lastRequest.serviceName, false).c_str()))
                        {
                            //create mainServiceCfg
                            #ifdef J54J6_LOGGING_H
                                logger logging;
                                logging.SFLog(className, "autoAddService", "Creating new Service Main CFG");
                            #endif

                            if(!FM->createFile(getFilename(lastRequest.serviceName).c_str()))
                            {
                                #ifdef J54J6_LOGGING_H
                                    logger logging;
                                    logging.SFLog(className, "autoAddService", "Can't create new File - create File returns false! - ERROR!", 2);
                                #endif
                                error.error = true;
                                error.ErrorCode = 378;
                                error.message = "Can't create new File - createFile() returns false!";
                                error.priority = 5;
                                lastRequest.reset();
                                autoAddRunning = false;
                                return 0;
                            }
                            else
                            {
                                bool ipAdded = FM->appendJsonKey(getFilename(lastRequest.serviceName).c_str(), "ip", lastFetched["ip"]);
                                bool macAdded = FM->appendJsonKey(getFilename(lastRequest.serviceName).c_str(), "mac", lastFetched["mac"]);
                                bool portAdded = FM->appendJsonKey(getFilename(lastRequest.serviceName).c_str(), "port", lastFetched["servicePort"]);

                                if(ipAdded && macAdded && portAdded)
                                {
                                    #ifdef J54J6_LOGGING_H
                                        logger logging;
                                        String message = "New Service Main CFG \"";
                                        message += lastRequest.serviceName;
                                        message += "\" successfully added - return true";
                                        logging.SFLog(className, "autoAddService", message.c_str());
                                    #endif

                                    String data = FM->readFile(getFilename(lastRequest.serviceName).c_str());

                                    //for debug
                                    /*
                                    Serial.println("--------------------------------");
                                    Serial.println(data);
                                    Serial.println("--------------------------------");
                                    */


                                    lastRequest.reset();
                                    autoAddRunning = false;

                                   
                                    return 1;
                                }
                                else
                                {
                                    #ifdef J54J6_LOGGING_H
                                        logger logging;
                                        String message = "New Service Main CFG \"";
                                        message += lastRequest.serviceName;
                                        message += "\" can't be added - Error while insert JSON- return false";
                                        logging.SFLog(className, "autoAddService", message.c_str(), 2);
                                    #endif
                                    lastRequest.reset();
                                    autoAddRunning = false;
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            /*
                                File already exist - check if configured too (contains any key/value) and end autoAdd
                            */
                            if(strcmp(FM->readJsonFileValue(getFilename(lastRequest.serviceName).c_str(), "serviceName"), "") != 0) //file has config
                            {
                                #ifdef J54J6_LOGGING_H
                                    logger logging;
                                    logging.SFLog(className, "autoAddService", "Can't create Servicefile - service already configured - stop AutoAdd - return true", 1);
                                #endif
                                autoAddRunning = 0;
                                return 1;
                            }
                            else
                            {
                                #ifdef J54J6_LOGGING_H
                                    logger logging;
                                    logging.SFLog(className, "autoAddService", "Can't create Servicefile - servicefile already exist - but not correct configured! - stop AutoAdd - return false", 2);
                                #endif
                                autoAddRunning = 0;
                                return 0;
                            }   
                        }
                    }
                    else 
                    {
                        /*
                            Creating Fallback Service CFG
                        */

                        if(!FM->fExist(getFilename(lastRequest.serviceName, true).c_str()))
                        {
                            //create mainServiceCfg
                            #ifdef J54J6_LOGGING_H
                                logger logging;
                                logging.SFLog(className, "autoAddService", "Creating new Service Fallback CFG");
                            #endif

                            if(!FM->createFile(getFilename(lastRequest.serviceName, true).c_str()))
                            {
                                #ifdef J54J6_LOGGING_H
                                    logger logging;
                                    logging.SFLog(className, "autoAddService", "Can't create new File - create File returns false! - ERROR!", 2);
                                #endif
                                error.error = true;
                                error.ErrorCode = 378;
                                error.message = "Can't create new File (fallback service) - createFile() returns false!";
                                error.priority = 5;
                                lastRequest.reset();
                                autoAddRunning = false;
                                return 0;
                            }
                            else
                            {
                                bool ipAdded = FM->appendJsonKey(getFilename(lastRequest.serviceName, true).c_str(), "ip", lastFetched["ip"]);
                                bool macAdded = FM->appendJsonKey(getFilename(lastRequest.serviceName, true).c_str(), "mac", lastFetched["mac"]);
                                bool portAdded = FM->appendJsonKey(getFilename(lastRequest.serviceName, true).c_str(), "port", lastFetched["servicePort"]);

                                if(ipAdded && macAdded && portAdded)
                                {
                                    #ifdef J54J6_LOGGING_H
                                        logger logging;
                                        String message = "New Service Fallback CFG \"";
                                        message += lastRequest.serviceName;
                                        message += "\" successfully added - return true";
                                        logging.SFLog(className, "autoAddService", message.c_str());
                                    #endif

                                    String data = FM->readFile(getFilename(lastRequest.serviceName, true).c_str());

                                    //for debug
                                    /*
                                    Serial.println("--------------------------------");
                                    Serial.println(data);
                                    Serial.println("--------------------------------");
                                    */

                                    lastRequest.reset();
                                    autoAddRunning = false;
                                    
                                    
                                    
                                    return 1;
                                }
                                else
                                {
                                    #ifdef J54J6_LOGGING_H
                                        logger logging;
                                        String message = "New Service Fallback CFG \"";
                                        message += lastRequest.serviceName;
                                        message += "\" can't be added - Error while insert JSON- return false";
                                        logging.SFLog(className, "autoAddService", message.c_str(), 2);
                                    #endif
                                    lastRequest.reset();
                                    autoAddRunning = false;
                                    return 0;
                                }
                            }
                        }
                        else
                        {
                            /*
                                File already exist - check if configured too (contains any key/value) and end autoAdd
                            */
                            if(strcmp(FM->readJsonFileValue(getFilename(lastRequest.serviceName, true).c_str(), "serviceName"), "") != 0)
                            {
                                #ifdef J54J6_LOGGING_H
                                    logger logging;
                                    logging.SFLog(className, "autoAddService", "Can't create Service Fallback file - service already configured - stop AutoAdd - return true", 1);
                                #endif
                                autoAddRunning = 0;
                                return 1;
                            }
                            else
                            {
                                #ifdef J54J6_LOGGING_H
                                    logger logging;
                                    logging.SFLog(className, "autoAddService", "Can't create Service Fallback file - servicefile already exist - but not correct configured! - stop AutoAdd - return false", 2);
                                #endif
                                autoAddRunning = 0;
                                return 0;
                            }   
                        }
                    }//create main or fallback cfg - else end - create fallback(else)
                    
                } //id of lastRequest and lastFetched are matching if ot - do nothing

            }//lastFetched contains all keys (else block) 

        }//check for correct id and searchType - if not match - do nothing   

    } //loop stuff - else
return 2;
} //autoAdd end

bool ExternServiceHandler::manAddService(const char* serviceName, int port, IPAddress ip, bool fallback, bool checkConnect, const char* MAC)
{
   return false;
}

bool ExternServiceHandler::delService(const char* serviceName)
{
    return false;
}


//helper functions
String ExternServiceHandler::getFilename(const char* serviceName, bool fallback)
{
    String cacheName = serviceName;
    cacheName.replace(" ", "");

    String rfilename = serviceAddressListFile;
    rfilename += cacheName;
    if(fallback)
    {
        rfilename += "-fallback";
    }
    rfilename += ".json";
    rfilename.replace(" ", "");
    return rfilename;
}



//loop
void ExternServiceHandler::loop()
{
    if(autoAddRunning)
    {
        autoAddService();
    }
    lastRequest.loop();
    networkIdent->loop();
}

/*
    Inherited overwritten functionalities
*/
void ExternServiceHandler::startClass()
{
    if(this->classDisabled)
    {
        this->classDisabled = false;
    }
}

void ExternServiceHandler::stopClass()
{
    if(!this->classDisabled)
    {
        this->classDisabled = true;
        Serial.println("locked!");
        return;
    }
}

void ExternServiceHandler::pauseClass()
{
    this->stopClass();
}

void ExternServiceHandler::restartClass()
{
    this->startClass();
}

void ExternServiceHandler::continueClass()
{
    this->startClass();
}