#include "mqttConfig.h"

ESPOS_Module_MQTTConfig::ESPOS_Module_MQTTConfig(MQTTHandler* mqtt, Filemanager* FM, WiFiManager* wifi)
{
    _MQTT = mqtt;
    _FM = FM; 
    _WiFi = wifi;
}


bool ESPOS_Module_MQTTConfig::getInformation(StaticJsonDocument<200> lastCb)
{
    Serial.println("In FKT");
    String message = lastCb["settings"][0][0];
    Serial.println("Start functuin " + message);
    std::function<String()> input; std::bind(input, message);
    Serial.println(input());
}



bool ESPOS_Module_MQTTConfig::checkFiles()
{
    static bool IsDoubleCheck = false;
    bool fileExist = _FM->fExist(this->configFile);
    if(fileExist)
    {
        bool hasMode = _FM->checkForKeyInJSONFile(configFile, "confMode");
        bool hasPass = _FM->checkForKeyInJSONFile(configFile, "password");
        bool hasChannel = _FM->checkForKeyInJSONFile(configFile, "mqttChannel");

        if(hasMode && hasPass && hasChannel)
        {
            logging.logIt("checkFiles", "Config File already exist", 3);
            return true;
        }
    }

    if(!fileExist)
    {
        if(!_FM->createFile(configFile))
        {
            logging.logIt("checkFiles", "Error while creating configFile", 4);
            return false;
        }
    }

    bool modeAdd = _FM->appendJsonKey(configFile, "confMode", "false");
    bool passAdd = _FM->appendJsonKey(configFile, "password", "");
    String path = "/control/config/";
    path += _WiFi->getDeviceMac();
    bool pathAdd = _FM->appendJsonKey(configFile, "mqttChannel", path.c_str());

    if(modeAdd && passAdd && pathAdd)
    {
        logging.logIt("checkFiles", "Successfully created config!", 3);
        if(!IsDoubleCheck)
        {
            IsDoubleCheck = true;
            bool res =  begin();
            if(res)
            {
                logging.logIt("checkFiles", "Config verified!", 3);
                return true;
            }
            logging.logIt("checkFiles", "Can't verify config!", 4);
            return false;
        }
        else
        {
            logging.logIt("checkFiles", "Can't verify config!", 4);
            return true;
        }

    }
    else
    {
        logging.logIt("checkFiles", "Error while adding Config!", 4);
    }
    return false;
}

bool ESPOS_Module_MQTTConfig::begin()
{
    logging.logIt("begin", "Init MQTT COnfig Module", 2);
    if(!checkFiles())
    {
        this->mqttConfigEnabled = false;
        logging.logIt("begin", "Can't init. Config Module - MQTTConfig is disabled!", 4);
        classControl.newReport("Can't start MQTT Config Module - Can't load Files", 404, 5, true);
        return false;
    }

    this->mqttConfigEnabled = _FM->returnAsBool(_FM->readJsonFileValue(configFile, "confMode"));
    this->basemqttConfigChannel = _FM->readJsonFileValue(configFile, "mqttChannel");
    this->passwd = _FM->readJsonFileValue(configFile, "password");

    if(mqttConfigEnabled)
    {
        logging.logIt("begin", "MQTT COnfig - active!", 2);
    }
    run();
}

bool ESPOS_Module_MQTTConfig::setPassword(const char* newPassword)
{
    bool resChg = _FM->changeJsonValueFile(configFile, "password", newPassword);
    bool resBegin = begin();
    if(resChg && resBegin)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool ESPOS_Module_MQTTConfig::setChannel(const char* newChannel)
{
    bool resChg = _FM->changeJsonValueFile(configFile, "mqttChannel", newChannel);
    bool resBegin = begin();
    if(resChg && resBegin)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool ESPOS_Module_MQTTConfig::setConfModeEnabled(bool confModeViaMQTTEnabled)
{
    String txt;
    if(confModeViaMQTTEnabled)
    {
        txt = "true";
    }
    else
    {
        txt = "false";
    }
    bool resChg = _FM->changeJsonValueFile(configFile, "confMode", txt.c_str());
    bool resBegin = begin();
    if(resChg && resBegin)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ESPOS_Module_MQTTConfig::run()
{
    if(!this->_MQTT->isConnected() || !this->mqttConfigEnabled)
    {
        return;
    }

    //MQTT is Connected and Conf is enabled

    static bool isSubscribed = false;

    if(!isSubscribed)
    {
        if(!this->_MQTT->subscribe(this->basemqttConfigChannel.c_str()))
        {
            logging.logIt("run", "Can't subscribe to Topic!", 4);
            return;
        }
        logging.logIt("run", "Subscribed to" + String(basemqttConfigChannel), 2);
        isSubscribed = true;
    }
    lastMqttCallback* lastCallback= _MQTT->getCallback();
    if( String(lastCallback->topic) == this->basemqttConfigChannel)
    {
        StaticJsonDocument<200> lastCb;
        logging.logIt("run", lastCallback->getPayload().c_str(), 2);
        DeserializationError error = deserializeJson(lastCb, lastCallback->getPayload());

        if(error)
        {

            return;
        }
        logging.logIt("run", "Received useable JSON", 2);

        if(passwd != "" || passwd != " ")
        {
            if(lastCb["passwd"] != passwd)
            {
                //sendError(1);
            }
        }

        if(lastCb["type"] == "get")
        {
            getInformation(lastCb);
        }
        else if(lastCb["type"] == "set")
        {
            //setInformation(lastCb);
        }
        else
        {
            //sendError();
        }

    }
}

