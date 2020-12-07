#ifndef MQTTHANDLER
#define MQTTHANDLER

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "wifiManager.h"
#include "filemanager.h"
#include "serviceHandler.h"

#include "errorHandler.h"
#include "logging.h"


/*
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
    const char* payload = "";
    const char* outputModuleName = "";

    void reset()
    {
        topic = "";
        payload = "";
        outputModuleName = "";
    }

    const char* getPayload(const char* moduleName = "")
    {
        if(outputModuleName == "" || strcmp(outputModuleName, moduleName))
        {
            return payload;
        }
    }

    const char* getTopic(const char* moduleName = "")
    {
        if(outputModuleName == "" || strcmp(outputModuleName, moduleName))
        {
            return topic;
        }
    }
};

class MQTTHandler : public ErrorSlave {
    private:
        Filemanager* FM;
        logger logging;
        WiFiManager* wifiManager;
        ServiceHandler* services;
        PubSubClient mqttHandlerClient;

        lastMqttCallback lastCallback;

        const char* configFallback[1][2] = {
            {"", ""}
        };
        int configFileLength = 0;

        const char* className = "MQTTHandler";

        const char* configFile = "/config/mqtt.json";

        int defaultPort = 1883;
        bool MQTTActive = false;
        bool classDisabled = false;
    
    protected:
        //extra Stuff
        bool configCheck();

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
        Connect
    */
    bool connect(); //try to connect to as external defined MQTT Server or fallback - if fail: return false
    bool connect(const char* id);
    bool connect(const char* id, const char* user, const char* pass);
    bool connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage, boolean cleanSession);

     /*
        Subscription Control
    */
   bool subscribe(const char* topic);
    //bool subscribe(const char* topic, uint8_t qos);

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
    virtual void eventListener(char* topic, byte* payload, unsigned int length);
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
    void pauseClass();
    void restartClass();
    void continueClass();     
};

#endif