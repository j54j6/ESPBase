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


bool ExternServiceHandler::autoAddService(const char* serviceName)
{
    if(!autoAddWait) //autoAddWait only because of the correct Log Message - theoretically i can check it here...
    {
        if(FM->fExist(getFilename(serviceName).c_str()))
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "autoAddService", "Service Cfg already exist - no autoConf needed! - return true", 1);
            #endif
            return true;
        }
        else
        {
            networkIdent->searchForService(serviceName);
            lastRequest.id = networkIdent->getLastGeneratedId();
            lastRequest.createdAt = millis();
            lastRequest.serviceName = serviceName;
            lastRequest.searchType = 3;
            autoAddWait = true;
            autoAddLastCall = millis();
        }
        
    }
    else
    {
        if(millis() < (lastRequest.createdAt + lastRequest.deleteAfter - 100))
        {
            
            if(millis() > (autoAddLastCall + checkDelay))
            {
                autoAddLastCall = millis();
                if(FM->fExist(getFilename(lastRequest.serviceName).c_str()))
                {
                    #ifdef J54J6_LOGGING_H
                        logger logging;
                        String message = "Service \"";
                        message += lastRequest.serviceName;
                        message += "\" successfully added";
                        logging.SFLog(className, "autoAddService", message.c_str(), 0);
                    #endif
                    lastRequest.reset();
                    autoAddWait = false;
                    return true;
                }
                return false;
            }
            else
            {
                delay(10);
                return false;
            }
            
        }
        #ifdef J54J6_LOGGING_H
            logger logging;
            String message = "Service \"";
            message += lastRequest.serviceName;
            message += "\" can't be added - timeout";
            logging.SFLog(className, "autoAddService", message.c_str(), 1);
        #endif
        lastRequest.reset();
        autoAddWait = false;
        return false;
    }
    return false;
}

bool ExternServiceHandler::manAddService(const char* serviceName, int port, IPAddress ip, bool fallback, bool checkConnect, const char* MAC)
{
   
}

bool ExternServiceHandler::delService(const char* serviceName)
{

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
    if(lastRequest.id != 0) //there is an request < 10 seconds old
    {
        StaticJsonDocument<425> lastFetched = networkIdent->getLastData();
        if(lastFetched.containsKey("type"))
        {
            if(lastFetched["type"] == "answer")
            {
                if(lastFetched.containsKey("id") && lastFetched.containsKey("serviceName"))
                {
                    String m1 = lastFetched["id"];
                    String m2 = lastFetched["serviceName"];
                    m1.replace(" ", "");
                    m2.replace(" ", "");

                    if(m1 == String(lastRequest.id) && m2 == String(lastRequest.serviceName))
                    {
                        if(lastFetched.containsKey("ip") && lastFetched.containsKey("mac") && lastFetched.containsKey("servicePort"))
                        {
                            //current request has the same id like last fech -> result will be saved in file as new Service address
                            if(!FM->fExist(getFilename(lastRequest.serviceName).c_str()))
                            {
                                //create first Service File
                                FM->fOpen(getFilename(lastRequest.serviceName).c_str(), "w");
                                FM->fWrite(" ");
                                FM->fClose();
                                FM->appendJsonKey(getFilename(lastRequest.serviceName).c_str(), "ip", lastFetched["ip"]);
                                FM->appendJsonKey(getFilename(lastRequest.serviceName).c_str(), "mac", lastFetched["mac"]);
                                FM->appendJsonKey(getFilename(lastRequest.serviceName).c_str(), "port", lastFetched["servicePort"]);
                                #ifdef J54J6_LOGGING_H
                                    logger logging;
                                    String message = "Main Cfg for Service \"";
                                    message += lastRequest.serviceName;
                                    message += "\" was added";
                                    logging.SFLog(className, "loop", message.c_str());
                                #endif
                                return;
                            }
                            if(!FM->fExist(getFilename(lastRequest.serviceName, true).c_str()))
                            {
                                //create Fallback Address File - only generated if manually poked or if first host can't be found
                                String ipOnFile = FM->readJsonFileValue(getFilename(lastRequest.serviceName).c_str(), "ip");
                                String fetchedIP = lastFetched["ip"];
                                ipOnFile.replace(" ","");
                                fetchedIP.replace(" ", "");
                                if(ipOnFile == fetchedIP)
                                {
                                    #ifdef J54J6_LOGGING_H
                                        logger logging;
                                        logging.SFLog(className, "loop", "Can't add new ext. Service Fallback Record - ip already registered as Main", 1);
                                    #endif
                                    return;
                                }
                                else
                                {
                                    FM->fOpen(getFilename(lastRequest.serviceName, true).c_str(), "w");
                                    FM->fClose();
                                    FM->appendJsonKey(getFilename(lastRequest.serviceName, true).c_str(), "ip", lastFetched["ip"]);
                                    FM->appendJsonKey(getFilename(lastRequest.serviceName, true).c_str(), "mac", lastFetched["mac"]);
                                    FM->appendJsonKey(getFilename(lastRequest.serviceName, true).c_str(), "port", lastFetched["servicePort"]);
                                    #ifdef J54J6_LOGGING_H
                                        logger logging;
                                        String message = "Fallback Cfg for Service \"";
                                        message += lastRequest.serviceName;
                                        message += "\" was added";
                                        logging.SFLog(className, "loop", message.c_str());
                                    #endif
                                    return;
                                }
                            }
                            else
                            {
                                //Nothing to do - fallback and main are configured - at this time only two ways are supported - updated later by an universal way
                            }
                            
                        }
                        else
                        {
                            //one or more keys are missing
                            #ifdef J54J6_LOGGING_H
                                logger logging;
                                logging.SFLog(className, "loop", "Can't add new ext. Service Record - one ore more keys are missing", 0);
                            #endif
                        }
                    }
                }
                else
                {
                    //Not useable for this lib - there is no id stored - maybe for another lib or program
                }
                
            }
            else
            {
                //last fetched was an request or non-syntax confirm - skip
            }
            
        }
        else
        {
            //Nothing to do - no "type" in last Fetch - maybe because of other program using networkIdent port or wrong useage of lib
        }
    }
    else 
    {
        /*
            Nothing to do - no requests
        */
    }
    

    if(autoAddWait)
    {
        autoAddService();
    }
    lastRequest.loop();
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