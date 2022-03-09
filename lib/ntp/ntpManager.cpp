#include "ntpManager.h"

NTPManager::NTPManager(Filemanager* FM, WiFiManager* wifi, bool setUpdateIntervall, int updateIntervall):utcOffsetInSeconds(3600) 
{
    this->FM = FM;
    this->wifi = wifi;
    if(!setUpdateIntervall)
    {
        this->ntpClient = new NTPClient(ntpUdp, "de.pool.ntp.org", utcOffsetInSeconds);
    }
    else
    {
        this->ntpClient = new NTPClient(ntpUdp, "de.pool.ntp.org", utcOffsetInSeconds, updateIntervall);
    }
    
}

bool NTPManager::begin()
{
    if(wifi->isConnected())
    {
        ntpClient->begin();
        this->beginSuccess = true;
        logging.logIt(F("begin"), F("Begin successfull"), 3);
        return true;
    }
    logging.logIt(F("begin"), F("WiFi is not connected - can't Begin!"), 4);
    return false;
}

bool NTPManager::updateTime()
{
    if(!wifi->isConnected())
    {
        logging.logIt(F("updateTime"), F("WiFi is not connected - can't update Time"), 4);
        return false;
    }
    /*
    if(ntpClient->forceUpdate())
    {
        logging.logIt("updateTime", "Successfully updated time!", 3);
        return true;
    }
    return false;
    */
    return ntpClient->forceUpdate();
}

int NTPManager::getDay()
{
    return this->ntpClient->getDay();
}

int NTPManager::getHours()
{
    return this->ntpClient->getHours();
}

int NTPManager::getMinutes()
{
    return this->ntpClient->getMinutes();
}

int NTPManager::getSeconds()
{
    return this->ntpClient->getSeconds();
}

ulong NTPManager::getEpochTime()
{
    return this->ntpClient->getEpochTime();
}

void NTPManager::run()
{
    if(!beginSuccess)
    {
        begin();
    }
    this->ntpClient->update();
}

