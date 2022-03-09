#pragma once
#ifndef J54J6_SysLogger
#define J54J6_SysLogger
#include <Arduino.h>
#include "filemanager.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

/*
    Logging class by j54j6

    This class is used to log Data in a predefined format

    Outputtypes:
        - Serial log
        - File Log
        - HTTP Stream (later implemented)
*/

/*
    Configuration
*/

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

/*
    Serial Logging
        uncomment if you want to use serial Logging
        if the class must begin Serial Communication than you need to uncomment serialBaudrate then the class start Serial Communication with defined baudrate
*/
#define logSerial
#define serialBaudrate 115200
#define serialFormatMessage

/*
    File Logging
        If you want to log into Files you need to uncomment "logInFile"
        
        The default path is "/var/log/logger.log" if you want to use a custom one you need to uncomment the logFilePath and define your own

*/
//#define logInFile //Set Comment quotes here and the code compiles without any problems

//#define singleLogFile false // if true (default) all log messages are saved in one File otherwise in the given directoy a file is created for each class
#define fileFormatMessage

#define pushMqttAsJSON
#define prioMQTTPublishing


//define TextSpaces for Formatted Messages
#define classNameSpace 15
#define functionnameSpace 20
#define priorityValueSpace 6

//permanentErrorLog




class SysLogger {
    private:
        String moduleClassName;
        Filemanager* FM = NULL;
        static PubSubClient* mqttClient;

        String mqttPublishTopic;
        String macAddress;
        int errorCounter = 0;

        
    protected:
        //return PriorityValue as "humanReadable" text
        String getLogLevel(short value);

        //format inputs to a simple readable format

        String getFormattedMessage(String functionName, String message, short priority);

        void initMQTT();
    public:
        static short serialLogLevel;
        static short fileLogLevel;
        static short mqttLogLevel;
        static bool serialLogging;
        static bool fileLogging;
        static bool mqttLogging;
        #ifndef logInFile
            SysLogger(const char* className);
            //SysLogger(Filemanager* FM, const char* className);
        #else
        #endif
        //SysLogger(Filemanager* FM, const char* className, MQTTHandler* mqtt, WiFiManager* wifi);
        SysLogger(Filemanager* FM, const char* className);
        SysLogger() {};
        ~SysLogger() {};

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
        void logIt(String function, String message, char priority = 3);
        //void logIt(String message, char priority);
        //void logIt(const char* message, char priority);

        /*
            LogLevel:
                logLevel meanings:
                - 0 -> noLogging
                - 1 -> trace
                - 2 -> debug
                - 3 -> info
                - 4 -> warn
                - 5 -> error 
                - 6 -> fatal

                - 7 -> all
            
            logType:
                - 1 -> Serial Logging
                - 2 -> FileLogging
                - 3 -> MQTT Logging
                - 4 -> UDP Logging
                - 5 -> HTTP Logging
        */
        void setLogLevel(short newLevel, short logType);
        void setLogging(bool logging, short logType);
        void setMqttClient(PubSubClient* mqttClient);
        int getErrorCounter() { return this->errorCounter;};
        String messageToJSON(String function, String message, char prio);
};
#endif