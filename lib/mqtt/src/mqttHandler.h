#ifndef MQTTHANDLER
#define MQTTHANDLER

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "wifiManager.h"
#include "filemanager.h"
#include "serviceHandler.h"
#include "moduleState.h"

class WiFiManager;
class ServiceHandler;
class ClassModuleSlave;
class SysLogger;
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

        const char* configFallback[1][2] = {
            {"", ""}
        };
        int configFileLength = 0;

        const char* className = "MQTTHandler";

        const char* configFile = "/config/mqtt.json";

        int defaultPort = 1883;
        bool MQTTActive = false;
        bool classDisabled = false;

        bool serviceAddDelayActive = false;
        ulong serviceAddDelayTimeout = 0;

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

    bool connectToService(bool main = true); //normaly used to connect to MQTT Server - try main and Backup CFG
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