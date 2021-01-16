#include "logger.h"

//Constructor
SysLogger::SysLogger(const char* className)
{
    this->moduleClassName = className;
}

SysLogger::SysLogger(Filemanager* FM, const char* className)
{
    this->FM = FM;
    this->moduleClassName = className;
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

        #ifdef logSerial
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

        #endif //logSerial
    }

    if((priority >= fileLogLevel && fileLogLevel != 0) || fileLogLevel == 7)
    {
        #ifdef logInFile
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
        #endif //logInFile
    }
}

