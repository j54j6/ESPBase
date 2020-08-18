#include "NetworkIdent.h"

//Constructor
NetworkIdent::NetworkIdent(WiFiManager* wifiManager, Filemanager* FM, const char* deviceName, int port)
{
    this->deviceIdentName = deviceName;
    this->port = port;
    this->wifiManager = wifiManager;
    this->FM = FM;
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
    udpListenerStarted = true;
    return true;
}

void NetworkIdent::stop()
{
    udpListenerStarted = false;
    udpHandler.stop();
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
            logging.SFLog(className, "checkForService", "Error while read JSON File -> FM -> ArduinoJson has Error!");
        #endif
        return false;
        
    }
}

bool NetworkIdent::addService(const char* newServiceName, int usedPort)
{
    if(checkForService(newServiceName))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "addService", "Can't add Service - already exist or other Error - see above at Info");
        #endif
        return false;
    }
    
    
}


void NetworkIdent::searchForDeviceName(const char* deviceName)
{
    
}


//loop
void NetworkIdent::loop()
{
    if(wifiManager->getWiFiState() == WL_CONNECTED && udpListenerStarted == true)
    {
        int packetSize = udpHandler.parsePacket();
        if(packetSize)
        {
            char buffer[512];
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
    return;
}