#pragma once
#ifndef J54J6_NTP
#define J54J6_NTP

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "wifiManager.h"
#include "logger.h"
#include "filemanager.h"

class NTPManager {
    private:
        Filemanager* FM;
        SysLogger logging = SysLogger("NTP");
        WiFiUDP ntpUdp;
        NTPClient* ntpClient;
        WiFiManager* wifi;
        
        const long utcOffsetInSeconds;
        bool beginSuccess = false;
        
    public:
        NTPManager(Filemanager* FM, WiFiManager* wifi, bool setUpdateIntervall = false, int updateIntervall = 0);
        bool begin();
        bool updateTime();
        int getDay();
        int getHours();
        int getMinutes();
        int getSeconds();
        String getFormattedTime();
        ulong getEpochTime();
        void stopNTP();
        void run();
};

#endif //J54J6_NTP