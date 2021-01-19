#ifndef J54J6_OTA
#define J54J6_OTA

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "filemanager.h"
#include "mqttHandler.h"
#include "moduleState.h"
#include "logger.h"
#include "network.h"


class OTA_Manager
{
    private:
        const char* configFile = "/config/ota/ota.json";
};
#endif