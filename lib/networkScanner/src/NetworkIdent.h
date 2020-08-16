#ifndef J54J6_NetworkIdent
#define J54J6_NetworkIdent
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


class NetworkIdent {
    private:
        const char* className = "NetworkIdent";
        const char* deviceIdentName = "notSet";
        const char* functionalities[];

        WiFiUDP udpHandler;
    public:
        NetworkIdent(const char* deviceName, int numberServices, );

        //set stuff

        //get stuff
        const char* getDeviceIdentName();
        
        
        //control
        void searchForDeviceName(const char* deviceName);
        void searchForFunctionalities(const char* functionalitiy);

        void returnMyDeviceName();
        void returnMyFuntionalitiy();
        void returnMyIP();
};

#endif