#ifndef dht11temp
#define dht11temp

#include <Arduino.h>
#include <DHT.h>

class dht11Temp {
    private:
        float actualTemp = 0.0;
        float actualHumid = 0.0;
        bool useComputedTemp = true;
        bool fahrenheit = false;
        bool begin = false;
        
        DHT* dht;
    public:
        dht11Temp(int dhtPin, bool isFahrenheit = false);
        float getTemp();
        float getHumidity();
        void setUseComputedTemp(bool newVal);

        void run();
};
#endif //dht11temp