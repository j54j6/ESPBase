#include "logger.h"
#include <ESP8266WiFi.h>

short SysLogger::serialLogLevel;
short SysLogger::fileLogLevel;
short SysLogger::mqttLogLevel;
bool SysLogger::serialLogging;
bool SysLogger::fileLogging;
bool SysLogger::mqttLogging;
PubSubClient* SysLogger::mqttClient = NULL;
//Constructor

SysLogger::SysLogger(const char* className)
{
    this->moduleClassName = className;
    this->serialLogLevel = 3;
    this->fileLogLevel = 5;
    this->mqttLogLevel = 2;
    this->serialLogging = true;
    this->fileLogging = false;
    this->mqttLogging = true;
    initMQTT();
}

SysLogger::SysLogger(Filemanager* FM, const char* className)
{
    this->moduleClassName = className;
    this->serialLogLevel = 3;
    this->fileLogLevel = 5;
    this->mqttLogLevel = 2;
    this->serialLogging = true;
    this->fileLogging = false;
    this->mqttLogging = true;
    this->FM = FM;
    initMQTT();
}

//protected
String SysLogger::getLogLevel(short value)
{
    switch(value) 
    {
        case 1:
            return "Trace";
            break;
        case 2:
            return "Debug";
            break;
        case 4:
            return  "Warn";
            break;
        case 5:
            return "Error";
            break;
        case 6:
            return "Fatal";
            break;
        default: //default is 3 = info
            return "Info";
            break;
    }
}

String SysLogger::getFormattedMessage(String functionName, String message, short priority)
{
    //Output Variable
    String outputMessage;
    //local variablePointer - pointer to the current letter per Variable in each for-loop
    //unsigned short localPointerCount = 0;
    //insert className with correct Space
    for(uint i = 0; i < classNameSpace; i++)
    {
        if(i < moduleClassName.length())
        {
            outputMessage += moduleClassName[i];
        }
        else
        {
            outputMessage += " ";
        }
    }
    //insert ">" for an optical seperation btw className and functionname
    outputMessage += "> ";
    for(uint i = 0; i < functionnameSpace; i++)
    {
        if(i < functionName.length())
        {
            outputMessage += functionName[i];
        }
        else
        {
            outputMessage += " ";
        }
    }


    //insert "|" for an optical seperation btw. functionname and Priority
    outputMessage += "| ";

    //reset the localPointer -> read next String beginning at Pos 0
    //localPointerCount = 0;

    //insert given Priority by defined Parameters
    String priorityAsText = getLogLevel(priority);
    for(uint i = 0; i < priorityValueSpace; i++)
    {
        if(i < priorityAsText.length())
        {
            outputMessage += priorityAsText[i];
        }
        else
        {
            outputMessage += " ";
        }
    }


    outputMessage += ">> ";
    outputMessage += message;

    //remove inputVal from Storage
    return outputMessage;
}

void SysLogger::initMQTT()
{
    this->macAddress = WiFi.macAddress();
    this->mqttPublishTopic = "espOS/devices/" + macAddress + "/log";
}
//public
/*
    logLevel
        The logLevel decide what information should be logged - the defualt value is 3 (only info and above will logged)

        All logLevel inherit the logLevel up next that means if you define logLevel (5) also (6) will be logged - 7 is an extra option and will not be considered if you dont set the logLevel to (7)

    logLevel meanings:
        - 0 -> noLogging
        - 1 -> trace
        - 2 -> debug
        - 3 -> info
        - 4 -> warn
        - 5 -> error 
        - 6 -> fatal

        - 7 -> all
*/
void SysLogger::logIt(String function, String message, char priority)
{
    if(priority >= 5)
    {
        errorCounter++;
    }
    if((priority >= serialLogLevel && serialLogLevel != 0) || serialLogLevel == 7)
    {

        if(serialLogging)
        {
            #ifdef serialBaudrate
                    if(!Serial)
                    {
                        Serial.begin(serialBaudrate);
                    } 
            #endif //serialBaudtate

            if(Serial)
            {
                #ifdef serialFormatMessage
                    Serial.println(getFormattedMessage(function, message, priority));
                #else
                {
                    Serial.print(function);
                    Serial.print(", ");
                    Serial.print(getLogLevel(priority));
                    Serial.print(", ");
                    Serial.print(message);
                }
                #endif //serialFormatMessage
            }

        }
    }

    if((priority >= fileLogLevel && fileLogLevel != 0) || fileLogLevel == 7)
    {
        if(fileLogging)
        {
            #ifdef logFilePath
                #ifdef singleLogFile
                    if(!FM->fExist(logFilePath))
                    {
                        FM->createFile(logFilePath);
                    }

                #ifdef fileFormatMessage
                    FM->writeInFile(logFilePath, getFormattedMessage(function, message, priority).c_str(), "a");
                #else
                    String message = function + ", " + getLogLevel(priority) + ", " + message;
                    FM->writeInFile(logFilePath, message.c_str(), "a");
                #endif
                #else
                    String logFile = logFilePath + moduleClassName + ".log";
                    if(!FM->fExist(logFile.c_str()))
                    {
                        FM->createFile(logFile.c_str());
                    }

                #ifdef fileFormatMessage
                    FM->writeInFile(logFile.c_str(), getFormattedMessage(function, message, priority).c_str(), "a");
                #else
                    String message = function + ", " + getLogLevel(priority) + ", " + message;
                    FM->writeInFile(logFile, message.c_str(), "a");
                #endif
                #endif //singleLogFile
            #endif //logInFilePath
        }
    }

    if((priority >= mqttLogLevel && mqttLogLevel != 0) || mqttLogLevel == 7)
    {
        if(mqttLogging && this->mqttClient != NULL)
        {
            #ifdef pushMqttAsJSON
                this->mqttClient->publish(mqttPublishTopic.c_str(), messageToJSON(function, message, priority).c_str());
            #else
                this->mqttClient->publish(mqttPublishTopic.c_str() ,getFormattedMessage(function, message, priority).c_str());
            #endif

            #ifdef prioMQTTPublishing
                String prioAppend = getLogLevel(priority);
                prioAppend.toLowerCase();
                String preparedTopic = mqttPublishTopic + "/" + prioAppend;
                #ifdef pushMqttAsJSON
                    this->mqttClient->publish(preparedTopic.c_str(), messageToJSON(function, message, priority).c_str());
                #else
                    this->mqttClient->publish(preparedTopic.c_str() ,getFormattedMessage(function, message, priority).c_str());
                #endif
            #endif
        }
    }    
}


void SysLogger::setLogLevel(short newLevel, short logType)
{
    switch(logType)
    {
        case 1:
            this->serialLogLevel = newLevel;
            break;
        case 2:
            this->fileLogLevel = newLevel;
            break;
        case 3:
            this->mqttLogLevel = newLevel;
            break;
        default:
            break;
    }
}

void SysLogger::setLogging(bool  logging, short logType)
{
    switch(logType)
    {
        case 1:
            this->serialLogging = logging;
            break;
        case 2:
            this->fileLogging = logging;
            break;
        case 3:
            this->mqttLogging = logging;
            break;
        default:
            break;
    }
}

void SysLogger::setMqttClient(PubSubClient* mqttClient)
{
    this->mqttClient = mqttClient;
}

String SysLogger::messageToJSON(String function, String message, char prio)
{
    function.replace("\"", "'");
    function.replace("\\", "DEC(92)");
    message.replace("\"", "'");
    message.replace("\\", "DEC(92)");
    String prior = getLogLevel(prio);
    String messageOutput = "{\"class\": \"" + moduleClassName + "\", \"function\": \"" + function + "\", \"priority\": \"" + prior + "\", \"message\" : \"" + message + "\"}";
    return messageOutput;
}