#ifndef ESPBASE_H
#define ESPBase_H

//include external non own Libaries
#include <Arduino.h>


//include own Libs
#include "button.h"
#include "errorHandler.h"
#include "externServiceHandler.h"
#include "filemanager.h"
#include "led.h"
#include "logging.h"
#include "j54j6Mqtt.h"
#include "network.h"
#include "NetworkIdent.h"
#include "udpManager.h"
#include "wifiManager.h"


class EspBase : public Button, public ErrorSlave, public ErrorHandler, public ExternServiceHandler, public Filemanager, public LED, public logger, public J54J6_MQTT, public Network, public udpManager, public WiFiManager{
    private:

    public:
        EspBase();
};
#endif