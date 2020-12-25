#ifndef J54J6_LOGGING_H
#define J54J6_LOGGING_H
#include <Arduino.h>

class logger {
    /*
        Logging class with two different and independent controlable Logtypes
            -> Serial Logging
            -> inFile Logging - creates a File and saves all Messages into this File - can read Out by function or directly per Hardware Connection
    */

    private:
        int baudrate = 115200;
        bool serialLogging = true;
        int logLevel = -1;



        static int getLogLevel()
        {
            logger logging; //create instance of itself to use non static functionalities of this class e.g readout Configparameter
            return int(logging.logLevel);
        }

        const char* getLevel(int prio)
        {
            switch(prio)
            {
                case -1:
                    return "DEBUG";
                case 1:
                    return "WARN";
                    break;
                case 2:
                    return "ERROR";
                    break;
                default:
                    return "INFO";
                    break;
            }
        }

    protected:
        void SerialLog(const char* className, const char* function, const char* message, const char* level) //Serial Log output 
        {
            if(serialLogging)
            {
                if(!Serial)
                {
                    Serial.begin(baudrate);
                }
             Serial.print(className);
                Serial.print(" | ");
                Serial.print(function);
                Serial.print(" - ");
                Serial.print(level);
                Serial.print("-> ");
                Serial.println(message);
            }
        }

    public:
        logger()
        {
            return;
        }

        static void SFLog(const char* className, const char* function, const char* message, int prio = 0)
        {
            logger logging; //create instance of itself to use non static functionalities of this class e.g readout Configparameter
         
            if(prio >= getLogLevel())
            {
                const char* level = logging.getLevel(prio); //getLeveltype (INFO (0) - WARN (1) - ERROR (2))
                
                logging.SerialLog(className, function, message, level); //Serial Logging
            }
        }
};
#endif