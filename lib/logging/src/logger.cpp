#include "logger.h"

//Constructor
#ifndef logInFile
SysLogger::SysLogger(const char* className)
{
    this->moduleClassName = className;
}
#else
//logInFile
SysLogger::SysLogger(Filemanager* FM, const char* className)
{
    this->FM = FM;
    this->moduleClassName = className;
}
#endif 
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
    //define TextSpaces 
    #define classNameSpace 20
    #define functionnameSpace 15
    #define priorityValueSpace 15
    
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
    if((priority < logLevel && logLevel != 7) || logLevel == 0)
    {
        return;
    }

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
                Serial.print(message);
                Serial.print(", ");
                Serial.print(priority);
            }
            #endif //serialFormatMessage
        }
            
    #endif //logSerial

    #ifdef logInFile
        #ifdef logFilePath
            #ifdef singleLogFile

            #else

            #endif //singleLogFile
        #endif //logInFilePath
    #endif //logInFile
}

