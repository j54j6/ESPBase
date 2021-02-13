#include "logger.h"
short SysLogger::serialLogLevel;
short SysLogger::fileLogLevel;
short SysLogger::mqttLogLevel;
bool SysLogger::serialLogging;
bool SysLogger::fileLogging;
bool SysLogger::mqttLogging;
//Constructor
SysLogger::SysLogger(const char* className)
{
    this->moduleClassName = className;
    this->serialLogLevel = 1;
    this->fileLogLevel = 5;
    this->mqttLogLevel = 2;
    this->serialLogging = true;
    this->fileLogging = false;
    this->mqttLogging = true;
}

SysLogger::SysLogger(Filemanager* FM, const char* className)
{
    this->moduleClassName = className;
    this->serialLogLevel = 1;
    this->fileLogLevel = 5;
    this->mqttLogLevel = 2;
    this->serialLogging = true;
    this->fileLogging = false;
    this->mqttLogging = true;
    this->FM = FM;
}

 SysLogger::SysLogger(Filemanager* FM, const char* className, MQTTHandler* mqtt, WiFiManager* wifi)
 {
    this->moduleClassName = className;
    this->serialLogLevel = 1;
    this->fileLogLevel = 5;
    this->mqttLogLevel = 2;
    this->serialLogging = true;
    this->fileLogging = false;
    this->mqttLogging = true;
    this->FM = FM;
    this->wifi = wifi;
    this->mqtt = mqtt;
}
//protected
String SysLogger::getLogLevel(short value)
{
    String res;
    switch(value) 
    {
        case 1:
            res = "Trace";
            break;
        case 2:
            res = "Debug";
            break;
        case 4:
            res =  "Warn";
            break;
        case 5:
            res = "Error";
            break;
        case 6:
            res = "Fatal";
            break;
        default: //default is 3 = info
            res = "Info";
            break;
    }
    return res;
}

String SysLogger::getFormattedMessage(String functionName, String message, short priority)
{
    //Output Variable
    String outputMessage;
    //local variablePointer - pointer to the current letter per Variable in each for-loop
    unsigned short localPointerCount = 0;
    //insert className with correct Space
    for(int i = 0; i < classNameSpace; i++)
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
    for(int i = 0; i < functionnameSpace; i++)
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
    localPointerCount = 0;

    //insert given Priority by defined Parameters
    String priorityAsText = getLogLevel(priority);
    for(int i = 0; i < priorityValueSpace; i++)
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


//public

void SysLogger::logIt(String function, String message, char priority)
{
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
