#include "udpManager.h"

udpManager::udpManager(WiFiManager* wifiManager, int port)
{
    this->listenPort = port;
}

udpManager::~udpManager()
{
    udpHandler.~WiFiUDP();
}

int udpManager::getListenPort()
{
    return listenPort;
}


int udpManager::getLocalOutPort()
{
    return udpHandler.localPort();
}

udpPacketResolve* udpManager::getLastUDPPacketLoop()
{
    return &lastContent;
}

bool udpManager::begin()
{
    if(udpListenerStarted)
    {
        logger logging;
        logging.SFLog(className, "begin", "UDP handler already started - SKIP");
        return true;
    }

    if(udpHandler.begin(this->listenPort) == 0)
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
            message += listenPort;
            logging.SFLog(className, "begin", message.c_str());
        #endif
    }
    
    udpListenerStarted = true;
    return true;
}


void udpManager::stop()
{
    udpHandler.stop();
}

void udpManager::sendUdpMessage(const char* workload, IPAddress ip, int port)
{
    if(wifiManager->getWiFiState() == WL_CONNECTED)
    {
        if(udpHandler.beginPacket(ip.toString().c_str(), port) == 1)
        {
            udpHandler.write(workload);
            udpHandler.endPacket();
        
            /*
            Serial.println("------------------------------");
            Serial.print("IP: ");
            Serial.println(ip.toString());
            Serial.print("Port: ");
            Serial.println(port);
            Serial.print("Workload: ");
            Serial.println(workload);
            Serial.println("------------------------------");
            */
        }
        else
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "sendUdpMessage", "Can't send UDP Message - beginPacket - return false (0)", 2);
            #endif
        }
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "sendUdpMessage", "Can't send UDP Message - no Network!", 2);
        #endif
    }
}



void udpManager::run()
{
    if(classDisabled)
    {
        return;
    }
    lastContent.resetPack();
    if(wifiManager->getWiFiState() == WL_CONNECTED && udpListenerStarted == true)
    {
        int packetSize = udpHandler.parsePacket();
        if(packetSize)
        {
            if(lastContent.remoteIP == WiFi.localIP())
            {
                return;
            }
            lastContent.paketSize = packetSize;
            lastContent.remoteIP = udpHandler.remoteIP();
            lastContent.remotePort = udpHandler.remotePort();

            char cacheUDP[512];
            int numbersReaded = udpHandler.read(cacheUDP, 512);
            if(numbersReaded > 0)
            {
                lastContent.udpContent[numbersReaded] = 0;
            }
            lastContent.udpContent = cacheUDP;
        }
    }

    //NetworkIdent is enabled when class is used - NetworkIdent part

    // for debugging
/*
    if(lastContent.udpContent != "NULL")
    {
        Serial.println(".............");
        Serial.print("address: ");
        Serial.println(lastContent.remoteIP.toString().c_str());
        Serial.print("Port: ");
        Serial.println(lastContent.remotePort);
        Serial.print("Packet Size: ");
        Serial.println(lastContent.paketSize);
        Serial.println(lastContent.udpContent);
        Serial.println(".............");
    }
   */ 
    return;
}


/*
    Inherited overwritten functionalities
*/
void udpManager::startClass()
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

void udpManager::stopClass()
{
    if(!this->classDisabled)
    {
        this->classDisabled = true;
        this->run();
        Serial.println("locked!");
        return;
    }
}

void udpManager::pauseClass()
{
    this->stopClass();
}

void udpManager::restartClass()
{
    this->startClass();
}

void udpManager::continueClass()
{
    this->startClass();
}