#include "NetworkIdent.h"

NetworkIdent::NetworkIdent(Filemanager* FM, WiFiManager* wifiManager)
{
    this->FM = FM;
    this->wifiManager = wifiManager;

    
}

bool NetworkIdent::beginListen()
{
    if(udpControl.begin())
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "beginListen", "NetworkIdent Service started");
        #endif
        return true;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "beginListen", "Can't start NetworkIdent - udpControl return false!", 2);
        #endif
        return false;
    }
    return false;
}

void NetworkIdent::stopListen()
{
    udpControl.stop();
}


bool NetworkIdent::checkForService(const char* serviceName)
{
    if(!FM->fExist(serviceListPath))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "checkForService", "Can't check for Service - serviceFile doesn't exist!", 2);
        #endif
        return false;
    }

    const size_t capacity = JSON_OBJECT_SIZE(25) + 400;
    DynamicJsonDocument cacheDocument(capacity);

    cacheDocument = FM->readJsonFile(serviceListPath);

    if(cacheDocument.containsKey(serviceName))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "checkForService", "Service exist! - return true");
        #endif
        return true;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "checkForService", "Can't find Service in serviceList - return false");
        #endif
        return false;
    }
}


bool NetworkIdent::addService(const char* serviceName, int port)
{
    FM->begin();
    if(FM->fExist(serviceListPath))
    {
        const size_t capacity = JSON_OBJECT_SIZE(25) + 400;
        DynamicJsonDocument cacheDocument(capacity);

        cacheDocument = FM->readJsonFile(serviceListPath);

        cacheDocument.add(serviceName);
        cacheDocument.add(port);

        FM->writeJsonFile(serviceListPath, cacheDocument);
        if(checkForService(serviceName))
        {
            return true;
        }
        else
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "addService", "An Error occured while adding the Service please check! -  Service can't be added", 2);
            #endif
            return false;
        }

    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "beginListen", "Can't check for Service - ServiceList File doesn't exist!", 1);
        #endif
        return false;
    }
}


bool NetworkIdent::delService(const char* serviceName)
{
    if(!FM->fExist(serviceListPath))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "delService", "Can't delete Service, serviceList doesn't exist!", 1);
        #endif
        return false;
    }

    if(!checkForService(serviceName))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "delService", "Can't delete Service, serviceList doesn't contains the specified Service!", 1);
        #endif
        return false;
    }

    const size_t capacity = JSON_OBJECT_SIZE(25) + 400;
    DynamicJsonDocument cacheDocument(capacity);

    cacheDocument = FM->readJsonFile(serviceListPath);

    cacheDocument.remove(serviceName);

    if(checkForService(serviceName))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            logging.SFLog(className, "delService", "An Error occured while deleting the Service - please check!", 2);
        #endif
        return false;
    }
    #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "delService", "Service successfully deleted!");
    #endif
    return true;
}


//send stuff

void NetworkIdent::sendServiceList(IPAddress ip, int port)
{
    String serviceListFileContent;
    if(FM->fExist(serviceListPath))
    {
        serviceListFileContent = FM->readFile(serviceListPath); //read out the raw File as const char*
    }
    else
    {
        serviceListFileContent = "noServices";
    }

    udpControl.sendUdpMessage(serviceListFileContent.c_str(), ip, port);
    
}

void NetworkIdent::sendIP(IPAddress ip, int port)
{
    udpControl.sendUdpMessage(WiFi.localIP().toString().c_str(), ip, port);
}

void NetworkIdent::sendHostname(IPAddress ip, int port)
{
    udpControl.sendUdpMessage(WiFi.hostname().c_str(), ip, port);
}

void NetworkIdent::sendMAC(IPAddress ip, int port)
{
    udpControl.sendUdpMessage(WiFi.macAddress().c_str(), ip, port);
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


String NetworkIdent::formatMessage(bool request, const char* serviceName, const char* MAC, const char* ip, int servicePort)
{
    /*
    {
        "type": "request",
        "serviceName" : "notSet",
        "mac" : "<<MACAddress>>",
        "ip" : "<<IPAddress>>",
        "servicePort" : "<<port>>"
    }
    */

    String output = "{\"type\" : \"";
    if(request)
    {
        output += "request";
    }
    else
    {
        output += "answer";
        
    }
    output += "\",";
    output += "\"serviceName\" : \"";
    output += serviceName;
    output += "\", ";
    output += "\"mac\" : \"";
    output += MAC;
    output += "\", ";
    output += "\"ip\" : \"";
    output += ip;
    output += "\", ";
    output += "\"servicePort\" : \"";
    output += servicePort;
    output += "\" ";
    output += "}";


    return output;
}

networkAnswerResolve NetworkIdent::getReceivedParameters(StaticJsonDocument<425>* lastLoopDoc)
{
    /*
        Blueprint of networkAnswerResolve

        bool changed = false;
        bool request = false; //if true this is an request, false its an Answer
        String serviceName = "n.S";
        String mac = "n.S";
        IPAddress ip = IPAddress(0,0,0,0);
        int servicePort = -1;

    */

   networkAnswerResolve outputDoc;

   outputDoc.changed = true;
   if(lastLoopDoc['type'] == "request")
   {
       outputDoc.request = true;
   }
   else
   {
       outputDoc.request = false;
   }

   outputDoc.serviceName = lastLoopDoc['serviceName'];
   outputDoc.mac =  lastLoopDoc['mac'];
   outputDoc.ip = lastLoopDoc['ip'];
   outputDoc.servicePort = lastLoopDoc['servicePort'];

}


void NetworkIdent::searchForService(const char* serviceName, IPAddress ip, int port)
{
    String message = formatMessage(true, serviceName);
    udpControl.sendUdpMessage(message.c_str(), ip, port);

}


void NetworkIdent::loop()
{
    udpControl.run();
    if(classDisabled)
    {
        return;
    }
    udpPacketResolve* lastResolve = udpControl.getLastUDPPacketLoop();

    if(lastResolve->udpContent == "NULL")
    {
        return;
    }

    StaticJsonDocument<425> cacheDocument;
    
    Serial.println("-------------------------");
    Serial.print("Message: ");
    Serial.println(lastResolve->udpContent);
    Serial.println("-------------------------");
    DeserializationError error = deserializeJson(cacheDocument, lastResolve->udpContent);

    if(error)
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            String message = "Can't parse last UDP Content to Json - Json returned: \n!";
            message += error.c_str();
            message += "\n";
            logging.SFLog(className, "loop", message.c_str());
        #endif
    }
    else
    {
        //readout of lastUDPPacketLop if there are any new Data
        getReceivedParameters(&cacheDocument);

    }
}



/*
    Inherited overwritten functionalities
*/
void NetworkIdent::startClass()
{
    if(this->classDisabled)
    {
        this->classDisabled = false;
        this->beginListen();
    }
    else
    {
        this->beginListen();
    }
}

void NetworkIdent::stopClass()
{
    if(!this->classDisabled)
    {
        this->classDisabled = true;
        this->stopListen();
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