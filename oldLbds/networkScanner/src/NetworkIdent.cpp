#include "NetworkIdent.h"

NetworkIdent::NetworkIdent(Filemanager* FM, WiFiManager* wifiManager)
{
    this->FM = FM;
    this->wifiManager = wifiManager;

    
}

bool NetworkIdent::beginListen()
{
    createConfigFile();
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
/*
    Serial.println(".......................");
    Serial.println(FM->readFile(serviceListPath));
    Serial.println(".......................");
*/
    if(cacheDocument.containsKey(serviceName))
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            String message = "Service \"";
            message += serviceName;
            message += "\" exist! - return true";
            logging.SFLog(className, "checkForService", message.c_str());
        #endif
        return true;
    }
    else
    {
        #ifdef J54J6_LOGGING_H
            logger logging;
            String message = "Can't find Service \"";
            message += serviceName;
            message += "\" in serviceList - return false";
            logging.SFLog(className, "checkForService", message.c_str());
        #endif
        return false;
    }
}


bool NetworkIdent::addService(const char* serviceName, const char* port)
{
    FM->begin();
    if(FM->fExist(serviceListPath))
    {
        if(checkForService(serviceName))
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                logging.SFLog(className, "addService", "Service already exist - SKIP", 0);
            #endif 
            return true;
        }
        const size_t capacity = JSON_OBJECT_SIZE(25) + 400;
        StaticJsonDocument<capacity> cacheDocument;

        //cacheDocument = FM->readJsonFile(serviceListPath);
        String data = FM->readFile(serviceListPath);

        if(!data.isEmpty())
        {
            deserializeJson(cacheDocument, data);
        }
        cacheDocument[serviceName] = port;

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
            logging.SFLog(className, "delService", "Can't delete Service, serviceList doesn't contains the specified Service! - SKIP", 1);
        #endif
        return true;
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
        return true;
    }
    return false;
}


String NetworkIdent::formatMessage(bool request, bool generateId, String serviceName, String MAC, String ip, String servicePort, String givenID)
{
    /*
    {
        "type": "request",
        "serviceName" : "notSet",
        "mac" : "<<MACAddress>>",
        "ip" : "<<IPAddress>>",
        "servicePort" : "<<port>>"
        "id" : "<<id>>" //id is randomGenerated by millis()+RandomNumber
    }
    */
    /*
    Serial.println("********************************");
    Serial.print("Request: ");
    Serial.println(request);
    Serial.print("GenerateID: ");
    Serial.println(generateId);
    Serial.print("serviceName: ");
    Serial.println(serviceName);
    Serial.print("MAC: ");
    Serial.println(MAC);
    Serial.print("IP: ");
    Serial.println(ip);
    Serial.print("servicePort: ");
    Serial.println(servicePort);
    Serial.print("GivenID: ");
    Serial.println(givenID);
    Serial.println("********************************");
    */


    long genId = millis() + random(20, 2145895698);
    lastId = genId;



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
    if(!generateId)
    {
        output += "\"";
        output += "}";
    }
    else
    {
        if(givenID == "n.S")
        {
            output += "\",";
            output += "\"id\" : \"";
            output += genId;
            output += "\"}";
        }
        else
        {
            output += "\",";
            output += "\"id\" : \"";
            output += givenID;
            output += "\"}";
        }
        
    }
    /*
    Serial.println("Output String: ");
    Serial.println(output);
    Serial.println("********************************");
    */
    return output;
}

StaticJsonDocument<425> NetworkIdent::getLastData()
{
    return cacheDocument;
}

ulong NetworkIdent::getLastGeneratedId()
{
    return lastId;
}


void NetworkIdent::searchForService(const char* serviceName, bool generateId, IPAddress ip, int port)
{
    String message;
    if(!generateId)
    {
        message = formatMessage(true, false, serviceName);
    }
    else
    {
        message = formatMessage(true, true, serviceName);
    }
    
    udpControl.sendUdpMessage(message.c_str(), ip, port);

}


void NetworkIdent::loop()
{
    //check for new Packets with UDPControl Class on <<NetworkIdentPort>>
    udpControl.run();

    //if this class is Disabled - end function
    if(classDisabled)
    {
        return;
    }

    //get new Packets if there are new ones - if nothing new there is an empty pack delivered
    udpPacketResolve* lastResolve = udpControl.getLastUDPPacketLoop();

    //If there is any new Data -> Content != NULL -> clean all fragments delivered with the protocol -> in this Case only char until packackgeSize[n] will be readed all other are removed
    if(lastResolve->udpContent != "NULL")
    {
        lastResolve->clean(); //delete fragments at the end of document
    }
    
    //If there are no new Data UDPContent=NULL -> end function
    if(lastResolve->udpContent == "NULL")
    {
        return;
    }
    
    
    //for debug only
    /*
    Serial.println("---------------------------------");
    Serial.println("UDP Content: ");
    Serial.println(lastResolve->udpContent);
    Serial.println("---------------------------------");
    */


    //create a Json Document from the String given in the last Resolve -> result saved in <<error>>
    DeserializationError error = deserializeJson(cacheDocument, lastResolve->udpContent);

    /*
        This part called if the incoming data doesnt have the correct syntax - most Error showing Serial is "incomplete Input"
        
        In this Case you only get a Warn and the function ends - if you use this port for another thins unlike NetworkIdent you can use the data properly
    */
    if(error)
    {
        
        #ifdef J54J6_LOGGING_H
            logger logging;
            String message = "Can't parse last UDP Content to Json - Json returned: \n!";
            message += error.c_str();
            message += "\n";
            logging.SFLog(className, "loop", message.c_str(), 1);
        #endif
        return;
    }
    else
    {

        //check for any constructions

        /*
            If you are here you have a correct Json Data package received
            

            Basicly this "else" block does:
                First check if the Json Package includes:
                    - serviceName key 
                    - type key
                If this is not included the function will end

                If both keys are existing:
                    Check if the "type" value is "request" or "answer"

                If type is an request (type = "request")
                    First check for own Services the device offer to others
                    If nothing found saved services will be searched - if there is an service Matching <<serviceName>> - this will be sended
                If type is an answer (type = "answer")
                    Check if ID is the same sended by NetworkIdent
                        if true:
                            try to add the new Service data
                        else
                            function ends
                    at last step:
                        Test for Service
        
        */
        if(!cacheDocument.containsKey("type") || !cacheDocument.containsKey("serviceName"))
        {
            //If the parsed Document does not contain "type" and "serviceName" - drop the package and end function

            return;
        }


        //cache the Servicename to remove space in Names - because of earlier Problems i prevent double Services or basic mistakes
        String serviceNameCached = cacheDocument["serviceName"];
        serviceNameCached.replace(" ", "");
        

        if(cacheDocument["type"] == "request")
        {
            /*
                request Section

                First check for self offered Services matching the Json.serviceName
                if not
                check for interal saved addresses matching the json.servername
            */

           if(checkForService(cacheDocument["serviceName"]))
            {
                #ifdef J54J6_LOGGING_H
                    
                    logger logging;
                    String message = "Service ";
                    message += serviceNameCached;
                    message += "exist - return true";
                    logging.SFLog(className, "loop", message.c_str(), 1);
                #endif
                
                String fmsg;
                if(!cacheDocument.containsKey("id"))
                {
                    fmsg = formatMessage(false, false, cacheDocument["serviceName"], WiFi.macAddress(), wifiManager->getLocalIP(), FM->readJsonFileValue(serviceListPath, serviceNameCached.c_str()));
                    udpControl.sendUdpMessage(fmsg.c_str(), udpControl.getLastUDPPacketLoop()->remoteIP, this->networkIdentPort);
                }
                else
                {
                    fmsg = formatMessage(false, true, cacheDocument["serviceName"], WiFi.macAddress(), wifiManager->getLocalIP(), FM->readJsonFileValue(serviceListPath, serviceNameCached.c_str()), cacheDocument["id"]);
                    udpControl.sendUdpMessage(fmsg.c_str(), udpControl.getLastUDPPacketLoop()->remoteIP, this->networkIdentPort);
                }
                /*
                Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
                Serial.println(fmsg);
                Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
                //udpControl.sendUdpMessage(formatMessage(false, false, cacheDocument["serviceName"], WiFi.macAddress().c_str(), wifiManager->getLocalIP().c_str(), FM->readJsonFileValue(serviceListPath, serviceNameCached.c_str())).c_str(), udpControl.getLastUDPPacketLoop()->remoteIP, this->networkIdentPort);
                */
            }
            else
            {
                #ifdef J54J6_LOGGING_H
                    logger logging;
                    String message = "Service ";
                    message += serviceNameCached;
                    message += "doesn't exist - return nothing";
                    logging.SFLog(className, "loop", message.c_str(), 1);
                #endif
                
            }
            
            


        }
        else if(cacheDocument["type"] == "answer")
        {
            //nothing to do at this time - you can use getLastData() to receive all data in your function
            return;
        }
        else
        {
            #ifdef J54J6_LOGGING_H
                logger logging;
                const char* cachedType = cacheDocument["type"];
                String message = "Syntax of UDP Packet is wrong - packet-type: \n!";
                message += "Type: ";
                message += cachedType;
                message += "\n";
                logging.SFLog(className, "loop", message.c_str(), 1);
            #endif
        }
        
    }
    return;
}



/*
    Inherited overwritten functionalities

    This are very Basic implementations and will be fixed / better implemented later
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