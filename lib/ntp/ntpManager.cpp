#include "ntpManager.h"

NTPManager::NTPManager(Filemanager* FM, WiFiManager* wifi, bool setUpdateIntervall, int updateIntervall) 
{
    this->FM = FM;
    this->wifi = wifi;
    if(!setUpdateIntervall)
    {
        this->ntpClient = new NTPClient(ntpUdp, "pool.ntp.org", utcOffsetInSeconds);
    }
    else
    {
        this->ntpClient = new NTPClient(ntpUdp, "pool.ntp.org", utcOffsetInSeconds, updateIntervall);
    }
    
}

bool NTPManager::begin()
{
    if(wifi->isConnected())
    {
        ntpClient->begin();
        this->beginSuccess = true;
        return true;
    }
    return false;
}

bool NTPManager::updateTime()
{
    if(!wifi->isConnected())
    {
        return false;
    }
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
    this->ntpClient->update();
}

