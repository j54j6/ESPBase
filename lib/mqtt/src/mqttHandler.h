#pragma once
#ifndef MQTTHANDLER
#define MQTTHANDLER

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "wifiManager.h"
#include "filemanager.h"
#include "serviceHandler.h"
#include "moduleState.h"
#include "logger.h"


/*
    Config Default Values


    MQTT Config Blueprint

    {
        "id" : "",
        "user" : "",
        "pass" : "",
        "willTopic" : "",
        "willQos" : "",
        "willRetain" : "",
        "willMessage" : "",
        "cleanSession" : ""
    }
*/

/*
  MQTT Callback 

    Topic: const char*
    lastPayload: const char*
*/

struct lastMqttCallback {
    
    const char* topic = "";
    String payload = "";
    const char* outputModuleName = "";

    void reset()
    {
        topic = "";
        payload = "";
        outputModuleName = "";
    }

    String getPayload(const char* moduleName = "")
    {
        if((outputModuleName && !outputModuleName[0]) || strcmp(outputModuleName, moduleName) == 0)
        {
            return payload;
        }
    }

    const char* getTopic(const char* moduleName = "")
    {
        if((outputModuleName && !outputModuleName[0]) || strcmp(outputModuleName, moduleName))
        {
            return topic;
        }
    }
};

class MQTTHandler {
    private:
        Filemanager* FM;
        SysLogger logging;
        WiFiManager* wifiManager;
        ServiceHandler* services;
        PubSubClient mqttHandlerClient;
        ClassModuleSlave classControl = ClassModuleSlave("MQTTHandler", 20);

        lastMqttCallback lastCallback;

        const char* configFallback[10][2] = {
            {"updateServer", "update.nodework.de"},
            {"uri", "/update"},
            {"servertoken", "unidentified"},
            {"serverpass", "unidentified!kgh49zuh89!3u903/U90kPOI9=U/89zIH"},
            {"port", "80"},
            {"updateSearch", "true"},
            {"autoUpdate", "true"},
            {"softwareVersion", "0.0.1"},
            {"checkDelay", "86400"}
        };
        int configFileLength = 0;

        const char* className = "MQTTHandler";

        const char* configFile = "/config/mqtt.json";

        int defaultPort = 1883;
        bool MQTTActive = false;
        bool classDisabled = false;

        bool removeConfigIfBrokerCantReached = false;

        //Connect - Auto vars
        bool connectLocked = false; //true if autoConnect can't connect to defined service and try to find service in Network or if the function is locked because of multiple errors during connecting
        bool lockReasonIsError = false;
        int serviceSearchTimeout = 2000; //Timeout after the class check if serviceAdd was successful

        ulong timeoutAfterSearchError = 300000; //this time the class will wait if mqtt Broker can't be reached for  "tryToConnectXTimes"
        short tryToConnectXTimes = 4; //How often the class should try to connect ot MQTT Broker until the class will wait for "timeoutAfterSearchError" until next connect
        short connectTries = 0; //connect counter - will set to 0 if the timer goues up
        ulong unlockAt = 0; //contains the Locktime the connect() can't be used
        bool connectOrError = false; //firstTime connect called after serviceadd this wil true - if can't connect error increment by one else all parameters are resetted
        String lastUsedIP = "0"; //used to identifiy if a new config is availiable / decide which config is needed

    protected:
        //extra Stuff
        bool configCheck();
        void lockConnect(String usedIP, bool isError = false);
        void checklockConnectTimer(); //added in loop
        void resetConnectLock();

        //connect to different serviceTypes
        /*
            return:
                0 = failed
                1 -> success
                2 -> no useable config  
        */
        short connectSelfOffered();
        short connectToExternalBroker(bool isFallback = false);

        /*
            Return:
                1 -> Success
                2 -> no useable Config found!
                3 -> Can't connect to broker
                4 -> start autoAdd
        */
        short connectToMQTTBroker();
        
    public:        
        MQTTHandler(Filemanager* FM, WiFiManager* wifiManager, ServiceHandler* serviceHandler);
        MQTTHandler();
        //Destruct
        
        ~MQTTHandler() {
            MQTTActive = false;
        };

         /*
        Set Stuff
    */
    bool setServer(IPAddress ip, uint16_t port, bool save = true, bool asFallback = false);

    void setCallback(MQTT_CALLBACK_SIGNATURE);

    void setClient(Client& client);

    void setStream(Stream& stream);

    void setKeepAlive(uint16_t keepAlive);

    void setSocketTimeout(uint16_t timeout);

    bool setBufferSize(uint16_t size);

    uint16_t getBufferSize();

    //helper Function - read out saved config and set as much as possible
    //bool autoConfig();


    /*
        Set Stuff for Module Conf
    */
    bool setId(const char* id);
    bool setUser(const char* user);
    bool setPass(const char* pass);
    bool setWillTopic(const char* willTopic);
    bool setWillQos(uint8_t willQos);
    bool setWillRetain(bool willRetain);
    bool setWillMessage(const char* willMessage);
    bool setCleanSession(bool cleanSession = true);

    /*
        Get Stuff
    */
    String getWillMessage();
    String getWillTopic();
    uint8_t getWillQos();
    bool getWillRetain();
    bool getCleanSession();
    ClassModuleSlave* getClassModuleSlave()
    {
        return &classControl;
    }
    PubSubClient* getPubSubClient()
    {
        return &this->mqttHandlerClient;
    }

    /*
        Connect
    */
    bool connect(bool onlyUseExternal = false, bool searchService = false); //try to connect to as external defined MQTT Server or fallback - if fail: return false
    bool connect(const char* id);
    bool connect(const char* id, const char* user, const char* pass);
    bool connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);

    /*
        connect() - helper
    */
    bool searchForServiceInNetwork(); //if connectToService() fails - this function tries to find another network device offering MQTT - by using Networkident and 
    bool setUserAndPass();
    /*
        Subscription Control
    */
    bool subscribe(const char* topic);
    bool subscribe(const char* topic, uint8_t qos);

    bool unsubscribe(const char* topic);

    /*
        Disconnect
    */
    void disconnect();


        /*
        Publishing
    */
   bool publish(const char* topic, const char* payload);
    /*
    bool publish(const char* topic, const char* payload, boolean retained);
    bool publish(const char* topic, const uint8_t * payload, unsigned int plength);
    bool publish(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained);

    bool publish_P(const char* topic, const char* payload, boolean retained);
    bool publish_P(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained);
    */

   /*
    // Start to publish a message.
    // This API:
    //   beginPublish(...)
    //   one or more calls to write(...)
    //   endPublish()
    // Allows for arbitrarily large payloads to be sent without them having to be copied into
    // a new buffer and held in memory at one time
    // Returns 1 if the message was started successfully, 0 if there was an error
    bool beginPublish(const char* topic, unsigned int plength, boolean retained);
    // Finish off this publish message (started with beginPublish)
    // Returns 1 if the packet was sent successfully, 0 if there was an error
    int endPublish();
    // Write a single byte of payload (only to be used with beginPublish/endPublish)
    virtual size_t write(uint8_t);
    // Write size bytes from buffer into the payload (only to be used with beginPublish/endPublish)
    // Returns the number of bytes written
    virtual size_t write(const uint8_t *buffer, size_t size);
    */

    /*
        Action
    */
    void eventListener(char* topic, uint8* payload, uint length);
    lastMqttCallback* getCallback();

    /*
        Other Stuff to get this working^^
    */
    bool isConnected();

    int state();
    void init();   

    //loop
    void run();

    //control class
    void startClass();
    void stopClass();
    void restartClass();  
};
#endif