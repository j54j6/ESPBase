#include "mqttConfig.h"

ESPOS_Module_MQTTConfig::ESPOS_Module_MQTTConfig(MQTTHandler* mqtt, Filemanager* FM, WiFiManager* wifi)
{
    _MQTT = mqtt;
    _FM = FM; 
    _WiFi = wifi;
}


String ESPOS_Module_MQTTConfig::formatResponse(String command, short state, String payload)
{
    String reply = "{\"type\":\"response\", \"source\": \"" + String(WiFi.macAddress()) + "\", \"requestedCommand\": \"" + command + "\", \"state\": \"" + String(state) + "\", \"payload\":\"" + payload + "\"}";
    logging.logIt("formatResponse", reply, 1);
    return reply;
}

/*
bool ESPOS_Module_MQTTConfig::getInformation(StaticJsonDocument<200> lastCb)
{
    Serial.println("In FKT");
    String message = lastCb["settings"][0][0];
    Serial.println("Start functuin " + message);
    std::function<String()> input; std::bind(input, message);
    Serial.println(input());
    return true;
}
*/

void ESPOS_Module_MQTTConfig::MQTTConfReadFile(lastMqttCallback* lastCb, String value)
{
    if(!_FM->fExist(value.c_str()))
    {
        _MQTT->publish(lastCb->topic.c_str(), formatResponse("readFile->" + value, 0, "File doesn't exist!").c_str());
        delay(50);
    }
    else
    {
        String FileContent = _FM->readFile(value.c_str());
        _MQTT->publish(lastCb->topic.c_str(), formatResponse("readFile->" + value, 0, FileContent).c_str());
        delay(50);
    }
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
    String path =  "espOS/devices/" + WiFi.macAddress();
    path += "/settings";
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
    settingsMQTTTopic = _FM->readJsonFileValue(configFile, "mqttChannel");

    this->passwd = _FM->readJsonFileValue(configFile, "password");

    if(mqttConfigEnabled)
    {
        
        logging.logIt("begin", "MQTT COnfig - active!", 2);

    }
    return true;
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

bool ESPOS_Module_MQTTConfig::setConfMode(bool confModeViaMQTTEnabled)
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
        this->mqttConfigEnabled = confModeViaMQTTEnabled;
        run(true);
        return true;
    }
    else
    {
        return false;
    }
}

void ESPOS_Module_MQTTConfig::runGetCommands(lastMqttCallback* lastCallback)
{
    String command = lastCb["setting"];
    command.toLowerCase();

    //Commands defined here
    if(command == "readfile")
    {
        String value = lastCb["filename"];
        logging.logIt("checkForGeneralConfigCommand", "Try to read File" + String(value), 2);
        MQTTConfReadFile(lastCallback, value);
    }
}
void ESPOS_Module_MQTTConfig::runSetCommands(lastMqttCallback* lastCallback)
{
    String command = lastCb["setting"];
    //Serial.print("Setting: ");
    //Serial.println(command);
    command.toLowerCase();
    if(command == "disablemqttconfig")
    {
        if(this->setConfMode(false))
        {
            logging.logIt("checkForModuleConfCommand", "MQTTConfig successfully disabled!");
        }
        else
        {
            logging.logIt("checkForModuleConfCommand", "Can't disable MQTTConfig! - The device is not safe to use!", 6);
            classControl.newReport("Error while disabling MQTTConfig mode!", 875, 1, true);
        }
    }
    if(command == "changeconfig")
    {
        if(this->mqttConfigEnabled)
        {
            if(lastCb.containsKey("filename") && lastCb.containsKey("key") && lastCb.containsKey("value"))
            {
                String filename = lastCb["filename"];
                String key = lastCb["key"];
                String value = lastCb["value"];

                if(!_FM->fExist(filename.c_str()))
                {
                    logging.logIt("runSetCommands", "File not found!", 2);
                    _MQTT->publish(lastCallback->topic.c_str(), formatResponse("changeConfig", 0, "File not found").c_str());
                    return;
                }
                if(!_FM->checkForKeyInJSONFile(filename.c_str(), key.c_str()))
                {
                    logging.logIt("runSetCommands", "append new Key to File", 2);
                    if(_FM->appendJsonKey(filename.c_str(), key.c_str(), value.c_str()))
                    {
                        logging.logIt("runSetCommands", "Key sucessfully appended", 2);
                        _MQTT->publish(lastCallback->topic.c_str(), formatResponse("changeConfig", 1, "Ok").c_str());
                    }
                    else
                    {
                        logging.logIt("runSetCommands", "Key could not be appended - See log", 2);
                        _MQTT->publish(lastCallback->topic.c_str(), formatResponse("changeConfig", 0, "Can't append key").c_str());
                    }
                }
                else
                {
                    if(_FM->changeJsonValueFile(filename.c_str(), key.c_str(), value.c_str()))
                    {
                        logging.logIt("runSetCommands", "Key sucessfully changed", 2);
                        _MQTT->publish(lastCallback->topic.c_str(), formatResponse("changeConfig", 1, "Ok").c_str());
                    }
                    else
                    {
                        logging.logIt("runSetCommands", "Can't change Key! - See log", 2);
                        _MQTT->publish(lastCallback->topic.c_str(), formatResponse("changeConfig", 0, "Can't change key").c_str());
                    }
                }
                
            }
            else
            {
                logging.logIt("checkForCommands", "One or more JSON Parameters are missing!", 4);
                return;
            }
        }
        else
        {
            logging.logIt("checkForCommands", "Configmode disabled!", 3);
            return;
        }
    }
}


void ESPOS_Module_MQTTConfig::checkForCommands(lastMqttCallback* lastCallback)
{
    const char* tempChannel = _FM->readJsonFileValue(configFile, "mqttChannel");
    /*
    Serial.print("TempChannel: ");
    Serial.println(tempChannel);
    Serial.print("Topic: ");
    Serial.println(lastCallback->getTopic());
    */
    if(lastCallback->getTopic() == tempChannel)
    {
        logging.logIt("checkForCommands", lastCallback->getPayload().c_str(), 2);
        DeserializationError error = deserializeJson(lastCb, lastCallback->getPayload());
        
        const char tError = error.code();
        if(tError != 0) // not OK
        {
            logging.logIt("checkForCommands", "Can't deserialize MQTT to JSON! - Error: " + String(error.code()) + String(", ") + String(error.c_str()), 4);
            return;
        }
        if(!lastCb.containsKey("target") || !lastCb.containsKey("type") || !lastCb.containsKey("password") || !lastCb.containsKey("setting"))
        {
            logging.logIt("checkForCommands", "One or more JSON Parameters are missing!", 4);
            return;
        }
        String deviceTarget = lastCb["target"];
        deviceTarget.replace(":", "");
        deviceTarget.toLowerCase();
        String deviceMacAddress = WiFi.macAddress();
        deviceMacAddress.replace(":", "");
        deviceMacAddress.toLowerCase();

        if((deviceTarget == deviceMacAddress || deviceTarget == "") && lastCb["password"] == this->passwd)
        {
            if(lastCb["type"] == "get")
            {
                runGetCommands(lastCallback);
            }
            else if(lastCb["type"] == "set")
            {
                runSetCommands(lastCallback);
            }
            else
            {
                logging.logIt("checkForCommands", "Recieved unknown Command! - return");
                return;
            }

        }
        lastCb.clear(); //After finishing all stuff - remove Document
    }
}

void ESPOS_Module_MQTTConfig::run(bool startListen)
{
    if(!this->_MQTT->isConnected() || !this->mqttConfigEnabled)
    {
        return;
    }

    //MQTT is Connected and Conf is enabled

    if(startListen && this->_MQTT->isConnected() && this->mqttConfigEnabled)
    {
        const char* tempChannel = _FM->readJsonFileValue(configFile, "mqttChannel");
        if(!this->_MQTT->subscribe(tempChannel))
        {
            logging.logIt("run", "Can't subscribe to Topic!", 4);
            classControl.newReport("Can't Subscribe to ConfTopic!", 421, 2, true);
            return;
        }
        else
        {
            logging.logIt("run", "Subscribed to", 3);
            logging.logIt("run", tempChannel, 3);
            //Serial.print("Free Heap: ");
            //Serial.println(ESP.getFreeHeap());
        }
        
    }
    lastMqttCallback* lastCallback= _MQTT->getCallback();
    if(lastCallback->getPayload() != "")
    {
        /*
        Serial.println("Content: +++++++++++++++++MQTT Config have received!");
        Serial.println(lastCallback->getPayload());

        Serial.println("#############PRE USE####################");
        Serial.print("Free HEAP: ");
        Serial.println(ESP.getFreeHeap());
        Serial.print("Free Stack: ");
        Serial.println(ESP.getFreeContStack());
        Serial.print("Free SketchSpace: ");
        Serial.println(ESP.getFreeSketchSpace());
        Serial.print("Heap Fragment: ");
        Serial.println(ESP.getHeapFragmentation());
        
        Serial.println("----------Data");
        Serial.print("Content: ");
        Serial.println(lastCallback->getPayload());
        Serial.print("Topic: ");
        Serial.println(lastCallback->getTopic());
        Serial.println("#####################END#####################");
        */
        //Functionaol Stuff

        checkForCommands(lastCallback);
        delay(20);
    }
}