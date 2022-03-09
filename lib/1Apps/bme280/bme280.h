#pragma once
#ifndef NW_BME280
#define NW_BME280


#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <Wire.h>


//#include <Adafruit_VEML6075.h>


#include "filemanager.h"
#include "logger.h"
#include "mqttHandlerV2.h"
#include "network.h"

#define BME280Config "config/bme280.json"

//BME280 Class Config
#define defaultBME280ClassMQTTServiceEnabled "true"
#define defaultBME280ClassHTTPServiceEnabled "true"
#define defaultBME280ClassHTTPUseHTTPSEnabled "false"
#define defaultBME280ClassCheckIntervall 60

class BME280 {
    private:
        Filemanager* _FM;

        #ifdef compileLoggingClassCritical
            SysLogger* logger;
        #endif
        MQTTHandlerV2* mqtt;
        Network* network;
        ESP8266WebServer* webserver;
    	Adafruit_BME280 bme;


        boolean httpServiceEnabled = true;
        boolean mqttServiceEnabled = true;
        boolean httpsEnabled = false;


        boolean beginSuccessfull = false;
        boolean beginMQTTSuccessfull = false;

        float lastTemperature = 0.0;
        float lastHumidity = 0.0;
        float lastPressure = 0.0;
        float lastAltitude = 0.0;

        boolean retainMessages = true;

        String mqttTopic;
        int checkIntervall = 10; //s

        #define SEALEVELPRESSURE_HPA 1013.25


        boolean loadConfig();
        void readValues();
        void addWebservice();

        //HTTP Services
        void sendTemperature();
        void sendTemperatureJson();
        void sendHumidity();
        void sendHumidityJson();
        void sendPressure();
        void sendPressureJson();
        void sendAltitude();
        void sendAltitudeJson();
        void sendDataJson();

        //MQTT
        void enableMQTTService();
        void sendMQTTMessage();
        void checkForMQTTAction();

        //Webserver stuff
        void handleConfigSite();
        void saveNewConfig();
    public:
        BME280(Network* network, MQTTHandlerV2* mqtt, Filemanager* _FM);

        void begin();

        void run();
};
#endif