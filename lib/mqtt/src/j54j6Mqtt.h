#ifndef J54J6_MQTT_H
#define J54J6_MQTT_H
#include <Arduino.h>
#include <MQTTClient.h>
#include <mqtt.h>

#include "wifiManager.h"
#include "filemanager.h"

#include "errorHandler.h"
#include "logging.h"

class J54J6_MQTT : public ErrorSlave {
    private:
        const char* configFallback[1][2] = {
            {"", ""}
        };
        int configFileLength = 0;
        
        const char* className = "MQTT";

        const char* configFile = "/config/mqtt.json";
        MQTTClient mqttHandlerClient;

        Filemanager* FM;
        WiFiManager* wifiManager;
        int default_port = 1883;
        bool MQTTActive = false;

    protected:
        //extra stuff
        bool configCheck();
        
    public:        
        J54J6_MQTT(Filemanager* FM, WiFiManager* wifiManager);

        //Init MQTTClient - start listener
        void begin(const char* hostname);
        void begin(const char* hostname, int port = 1883);


        //Destruct
        ~J54J6_MQTT() {
            MQTTActive = false;
        };
        
        //Get Stuff
        bool connected();


        //Set Stuff
        void setHost(const char* hostname[]);
        void setHost(const char* hostname[], int port = 1883);

        void setWill(const char topic[]);
        void setWill(const char topic[], const char payload[]);
        void setWill(const char topic[], const char payload[], bool retained, int qos);
        void clearWill();

        void setKeepAlive(int keepAliveTime);
        void setCleanSession(bool cleanSession);
        void setTimeout(int timeout);
        void setOptions(int keepAlive, bool cleanSession, int timeout);

        void setClockSource(MQTTClientClockSource);


        
        //Response stuff
        void (*MQTTClientCallbackSimple)(String &topic, String &payload);
        void setOnMessage(String &topic, String &payload);


        //connection
        bool connect(const char clientId[], bool skip = false);
        bool connect(const char clientId[], const char username[], bool skip = false);
        bool connect(const char clientId[], const char username[], const char password[], bool skip = false);


        //publish
        bool publish(const String &topic);
        bool publish(const char topic[]);
        bool publish(const String &topic, const String &payload);
        bool publish(const String &topic, const String &payload, bool retained, int qos);
        bool publish(const char topic[], const String &payload);
        bool publish(const char topic[], const String &payload, bool retained, int qos);
        bool publish(const char topic[], const char payload[]);
        bool publish(const char topic[], const char payload[], bool retained, int qos);
        bool publish(const char topic[], const char payload[], int length);
        bool publish(const char topic[], const char payload[], int length, bool retained, int qos);

 
        //Subscribe
        bool subscribe(const String &topic);
        bool subscribe(const String &topic, int qos); 
        bool subscribe(const char topic[]);
        bool subscribe(const char topic[], int qos);


        //unsubscribe
        bool unsubscribe(const String &topic);
        bool unsubscribe(const char topic[]);


        //disconnecting
        bool disconnect();


        //debug
        lwmqtt_err_t lastError();
        lwmqtt_return_code_t returnCode();
        

        //loop
        void run();


        //control class
        void startClass();
        void stopClass();
        void pauseClass();
        void restartClass();
        void continueClass();        
};


#endif