#include "NetworkIdent.h"

//Constructor
NetworkIdent::NetworkIdent(WiFiManager* wifiManager, Filemanager* FM, const char* deviceName, int port)
{
    this->deviceIdentName = deviceName;
    this->port = port;
    this->wifiManager = wifiManager;
    this->FM = FM;

    this->begin(networkIdentPort);
}

//set stuff
void NetworkIdent::setDeviceName(const char* newDeviceName)
{
    this->deviceIdentName = newDeviceName;
}



//get Stuff
const char* NetworkIdent::getDeviceIdentName()
{
    return this->deviceIdentName;
}

int NetworkIdent::getLocalOutPort()
{
    return udpHandler.localPort();
}

udpPacketResolve* NetworkIdent::getLastUDPPacketLoop()
{
    return &lastContent;
}

//control
bool NetworkIdent::begin(int port)
{
    createConfigFile();
    if(udpHandler.begin(this->port) == 0)
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "begin", "Can't start UDP - begin() return false!", 2);
        #endif
        error.error = true;
        error.ErrorCode = 421;
        error.message = "Can't start UDP - UDP.begin() return false (0)!";
        error.priority = 5;
        return false;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            String message = "UDP handler started - ";
            message += "Listening on Port: ";
            message += port;
            logging.SFLog(className, "begin", message.c_str());
        #endif
    }
    
    udpListenerStarted = true;
    return true;
}

void NetworkIdent::stop()
{
    udpListenerStarted = false;
    udpHandler.stop();
}


bool NetworkIdent::createConfigFile()
{
    FM->mount();
    if(!FM->fExist(serviceListPath))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createConfigFile", "Config File doesn't exist - try to create", 1);
        #endif 

        if(FM->createFile(serviceListPath))
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "createConfigFile", "File created!");
            #endif 
        }
        else
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "createConfigFile", "Can't create File! - return false", 2);
            #endif
            return false;
        }

        if(FM->writeJsonFile(serviceListPath, serviceConfigBlueprint, 1))
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "createConfigFile", "Config Fallback written - return true");
            #endif
            return true;
        }
        else
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "createConfigFile", "Can't write in File!");
            #endif
            return false;
        }
        return false;  
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "createConfigFile", "File already exist - SKIP!");
        #endif
    }
    return false;
}


bool NetworkIdent::checkForService(const char* serviceName)
{
    if(FM->begin())
    {
        if(!FM->fExist(serviceListPath))
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "checkForService", "Servicelist File doesn't exist! - return False", 1);
            #endif
            return false;
        }
        
        DynamicJsonDocument serviceList(425);
        serviceList = FM->readJsonFile(serviceListPath);

        if(!FM->error)
        {
            return serviceList.containsKey(serviceName);
        }
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "checkForService", "Error while read JSON File -> FM -> ArduinoJson has Error!", 2);
        #endif
        FM->error = false;
        return false;
        
    }
    return false;
}

bool NetworkIdent::addService(const char* newServiceName, int usedPort)
{
    if(checkForService(newServiceName))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "addService", "Can't add Service - already exist or other Error - see Info above!");
        #endif
        return false;
    }
    
    DynamicJsonDocument services(425);
    services = FM->readJsonFile(serviceListPath);

    if(FM->error)
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "addService", "Error while read JSON File -> FM -> ArduinoJson has Error!", 2);
        #endif
        FM->error = false;
        return false;
    }
    services.add(newServiceName);
    services.add(usedPort);
    FM->writeJsonFile(serviceListPath, services);
    
    if(checkForService(newServiceName))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "addService", "Service successfully added");
        #endif
        return true;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "addService", "Can't add Service - Service not  found after Add!", 2);
        #endif
        return false;
    }
    return false;
}


bool NetworkIdent::delService(const char* serviceName)
{
    if(checkForService(serviceName))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "delService", "Service exist - try to delete it");
        #endif
        DynamicJsonDocument services(425);
        services = FM->readJsonFile(serviceListPath);

        if(FM->error)
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "delService", "Error while read JSON File -> FM -> ArduinoJson has Error!", 2);
            #endif
            FM->error = false;
            return false;
        }

        services.remove(serviceName);
        if(FM->writeJsonFile(serviceListPath, services))
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "delService", "Service deleted!", 0);
            #endif
            return true;
        }
        else
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "delService", "Can't write back File!", 2);
            #endif
            return false;
        }
        
    }
    return false;
}


void NetworkIdent::searchForDeviceName(const char* deviceName)
{
    if(wifiManager->getWiFiState() == WL_CONNECTED)
    {
        String message = "{\"deviceNameSearch\": \"";
        message += deviceName;
        message += "\"}";

        udpHandler.beginPacket(broadcastIP, networkIdentPort);
        udpHandler.write(message.c_str());
        udpHandler.endPacket();
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "searchForDeviceName", "Can't send UDP Message - no Network!", 2);
        #endif
    }
}

void NetworkIdent::searchForService(const char* serviceName)
{
    if(wifiManager->getWiFiState() == WL_CONNECTED)
    {
        String message = "{\"deviceServiceSearch\": \"";
        message += serviceName;
        message += "\"}";

        udpHandler.beginPacket(broadcastIP, networkIdentPort);
        udpHandler.write(message.c_str());
        udpHandler.endPacket();
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "searchForService", "Can't send UDP Message - no Network!", 2);
        #endif
    }
}


void NetworkIdent::sendUdpMessage(const char* workload, IPAddress ip, int port)
{
    if(wifiManager->getWiFiState() == WL_CONNECTED)
    {
        udpHandler.beginPacket(ip, port);
        udpHandler.write(workload);
        udpHandler.endPacket();
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "sendUdpMessage", "Can't send UDP Message - no Network!", 2);
        #endif
    }
    
    
}

void NetworkIdent::returnMyDeviceName()
{
    if(wifiManager->getWiFiState() == WL_CONNECTED)
    {
        String message = "{\"Answer\": \"";
        message += "\"TRUE\",";
        message += "\"DeviceName\":\"";
        message += this->deviceIdentName;
        message += "\",";
        message += "\"ip\":";
        message += "\"";
        message += wifiManager->getLocalIP();
        message += "\"}";


        udpHandler.beginPacket(broadcastIP, networkIdentPort);
        udpHandler.write(message.c_str());
        udpHandler.endPacket();
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "searchForDeviceName", "Can't send UDP Message - no Network!", 2);
        #endif
    }
}

void NetworkIdent::returnMyIP()
{
    if(wifiManager->getWiFiState() == WL_CONNECTED)
    {
        String message = "{\"Answer\": \"";
        message += "\"TRUE\",";
        message += "\"IP\":";
        message += "\"";
        message += wifiManager->getLocalIP();
        message += "\"}";


        udpHandler.beginPacket(broadcastIP, networkIdentPort);
        udpHandler.write(message.c_str());
        udpHandler.endPacket();
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "searchForDeviceName", "Can't send UDP Message - no Network!", 2);
        #endif
    }
}

//loop
void NetworkIdent::loop()
{
    if(classDisabled)
    {
        return;
    }

    if(wifiManager->getWiFiState() == WL_CONNECTED && udpListenerStarted == true)
    {
        int packetSize = udpHandler.parsePacket();
        if(packetSize)
        {
            lastContent.paketSize = packetSize;
            lastContent.remoteIP = udpHandler.remoteIP();
            lastContent.remotePort = udpHandler.remotePort();

            int numbersReaded = udpHandler.read(lastContent.udpContent, 512);
            if(numbersReaded > 0)
            {
                lastContent.udpContent[numbersReaded] = 0;
            }
        }
    }

    //NetworkIdent is enabled when class is used - NetworkIdent part
    if(lastContent.udpContent == "webserver")
    {
        Serial.println("Webserver send!");
    }
    return;
}


/*
    Inherited overwritten functionalities
*/
void NetworkIdent::startClass()
{
    if(this->classDisabled)
    {
        this->classDisabled = false;
        this->begin();
    }
    else
    {
        this->begin();
    }
}

void NetworkIdent::stopClass()
{
    if(!this->classDisabled)
    {
        this->classDisabled = true;
        this->loop();
        Serial.println("locked!");
        return;
    }
}

void NetworkIdent::pauseClass()
{
    this->stopClass();
}

void NetworkIdent::restartClass()
{
    this->startClass();
}

void NetworkIdent::continueClass()
{
    this->startClass();
}