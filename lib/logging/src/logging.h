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
        int logLevel = 0;



        static int getLogLevel()
        {
            logger logging; //create instance of itself to use non static functionalities of this class e.g readout Configparameter
            return int(logging.logLevel);
        }

        const char* getLevel(int prio)
        {
            switch(prio)
            {
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


/*

    
    class logger {
        private:
        
        //    If Filemanager availiable use config File defined below 
        
        #ifdef J54J6_FILEMANAGER_H
            const char* configFile = "logger.config"; //for later usage
            Filemanager FM;
        #endif


        static const int baudrate = 115200; //defined baudrate can't be changed (hardcoded) - modify this class manual - maybe changeable in ater Update

            //logging Class can be used with Filemanager to save and change attributes per File and setup during normal Work e.g per webinterface
            #ifdef J54J6_FILEMANAGER_H     
                
                const char* logLevel;
                const char* fileLogging;
                const char* serialLogging;
            #else
                
                bool serialLogging = true;
                int loglevel = 0;
            #endif

            
            //    before useage of class (object created in static Method) check for config File if Filemanager is integrated...
            
            logger(){
                #ifdef J54J6_FILEMANAGER_H
                    Serial.println("Filesystem information");
                    Serial.println("------------------------");
                    Serial.print("Content of configFile: ");
                    Serial.println(FM.readFile(configFile));
                    Serial.println("......................");
                    checkConfig(configFile);
                    this->logLevel = FM.readJsonFileValue(configFile, "loglevel");
                    this->serialLogging = FM.readJsonFileValue(configFile, "serialLogging");
                    this->fileLogging = FM.readJsonFileValue(configFile, "fileLogging");
                    this->FM = FM;
                #endif
            }

            logger()
            {

            }

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

        
        //    Only include FileLogging if fileManager is included 

        //    fileLogging can be set in logger.config saved as JSON
        
        #ifdef J54J6_FILEMANAGER_H 
            void writeInFileLog(const char* className, const char* function, const char* message, const char* level)
            {
                
                if(fileLogging == "true")
                {
                    if(!Serial)
                    {
                        Serial.begin(baudrate);
                    }
                    Serial.println("Write in File...");
                }
                return; //comming soon...
            }

            void checkConfig(const char* configFile)
            {
                Serial.println("create config!");
                Serial.print("Filemanager: ");
                Serial.println(FM.mount());

                const char* defaultConfig[3][2] = {
                    {"fileLogging", "false"},
                    {"serialLogging", "true"},
                    {"loglevel", "0"}
                };
                if(!FM.fExist(configFile))
                {
                    Serial.println("File doesn#t exist!");
                    if(FM.createFile("/logger.config"))
                    {
                        FM.writeJsonFile(configFile, defaultConfig, 2, "w");
                        Serial.println("Config created");
                    }
                    else{
                        Serial.println("Cant create File!");
                    }
                }
                else
                {
                    Serial.println("File already exist!.. SKIP");
                }
            }

            int getLogLevel()
            {
                logger logging; //create instance of itself to use non static functionalities of this class e.g readout Configparameter

                #ifdef J54J6_FILEMANAGER_H
                const char* logLevel = FM.readJsonFileValue(logging.configFile, "loglevel");
                #endif
                return int(logLevel);
            }
        #endif



            
               // Return the Level as "text"
            
            const char* getLevel(int prio)
            {
                switch(prio)
                {
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



        public:
            
            //    Public Logging function - only way to use this little Libary
            //        Simply use logger::SFLog(className, functuion, message, prio (default = 0)
            

            static void SFLog(const char* className, const char* function, const char* message, int prio = 0)
            {
                logger logging; //create instance of itself to use non static functionalities of this class e.g readout Configparameter


                
                if(prio >= int(logLevel))
                {
                    const char* level = logging.getLevel(prio); //getLeveltype (INFO (0) - WARN (1) - ERROR (2))
                    
                #ifdef J54J6_FILEMANAGER_H                              //Call WriteInFile only if filehandling Libary is ncluded to prevent errors and unuseable Code
                    
                    Serial.println("Write In File");
                    logging.writeInFileLog(className, function, message, level); 

                #endif
                    logging.SerialLog(className, function, message, level); //Serial Logging
                }
            }

            static void setLogLevel(int newLoglevel)
            {
                logger logging;
                #ifdef J54J6_FILEMANAGER_H
                    Serial.print("Change JSON loglevel: ");
                    Serial.println(FM->changeJsonValueFile(logging.configFile, "logLevel", String(newLoglevel).c_str()));
                #else
                    logging.loglevel = newLoglevel;
                #endif
            }                                                                                                                 

        #ifdef J54J6_FILEMANAGER_H
            static void setfileLogging(const char* newFileLogging)
            {
                #ifdef J54J6_FILEMANAGER_H
                    logger logging = logger();
                    Serial.print("Change JSON setFileLogging: ");
                    Serial.println(FM.changeJsonValueFile(logging.configFile, "fileLogging", newFileLogging));
                #else
                    fileLogging = newFileLogging;
                #endif
            }
        #endif

            static void setSerialLogging(bool newSerialLogging)
            {
                logger logging;
                #ifdef J54J6_FILEMANAGER_H
                    
                    Serial.print("Change JSON setSerialLogging: ");
                    Serial.println(FM.changeJsonValueFile(logging.configFile, "serialLogging", String(newSerialLogging).c_str()));
                #else
                    logging.serialLogging = newSerialLogging;
                #endif
            }
    };
#endif

*/