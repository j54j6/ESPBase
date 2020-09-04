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
        
    }
    
}

IPAddress ExternServiceHandler::getServiceIP(const char* serviceName)
{

}

bool ExternServiceHandler::autoAddService(const char* serviceName)
{

}

bool ExternServiceHandler::manAddService(const char* serviceName, int port, IPAddress ip, bool checkConnect, const char* MAC)
{

}

bool ExternServiceHandler::delService(const char* serviceName)
{

}


//helper functions
String ExternServiceHandler::getFilename(const char* serviceName)
{
    String cacheName = serviceName;
    cacheName.replace(" ", "");

    String rfilename = serviceAddressListFile;
    rfilename += cacheName;
    rfilename += ".json";
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