#ifndef GENERAL_H
#define GENERAL_H

#include <Arduino.h>
//int baudrate = 115200;

bool factoryReset();
/*
void writeSerialLog(const char* className, const char* function, const char* message, const char* level)
{
    Serial.print(className);
    Serial.print(" | ");
    Serial.print(function);
    Serial.print(" - ");
    Serial.print(level);
    Serial.print("-> ");
    Serial.println(message);
}

void SerialLog(const char* className, const char* function, const char* message, int prio = 0)
{
    if(!Serial)
    {
        Serial.begin(baudrate);
    }
    if(debugging == "true")
    {
        const char* level;
        switch(prio)
        {
            case 1:
                level = "WARN";
                break;
            case 2:
                level = "ERROR";
                break;
            default:
                level = "INFO";
                break;
        }
        switch(debugLevel)
        {
            case 1: //all Messages
                writeSerialLog(className, function, message, level);
                break;

            case 2: //only WARN and ERROR
                if(prio > 0)
                {
                    writeSerialLog(className, function, message, level);
                }
                break;
            case 3: //only ERROR
                if(prio > 1)
                {
                    writeSerialLog(className, function, message, level);
                }
                break;
            default:
                writeSerialLog("General", "log", "please only use debuglevel 1 - 3!", "INFO");
                break;
        }
    }
}
*/
#endif