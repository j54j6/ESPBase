#pragma once
#ifndef voltageDetect
#define voltageDetect

#include <Arduino.h>
/*      ____      __________________________A0
+ -----|____|----|--------------|           |_________
         R1(22K)       Vout     |           |°       |
                                |           |  esp   |
                                |           |________|
                                |           |GND
         ____                   |           |
- ------|____|-------------------           |                    
    |    R2(47K)                            |
    |_______________________________________|

*/
class voltageDetector {
    private:
        float factor = 0.00476861397;
        int voltagePin = -1;
        float actualValue = 1000;

        /*
            Batterytypes:
                0 => 18650 3,7V
        */
        int batteryType = 0;
    public:
        voltageDetector(int voltagePin = -1, int batteryType = 0, float powerFactor = 0.00476861397);
        float getActualVoltage();
        float getActualRawValue();
        int calcPercentage18650Battery();
        int getPercent();
        void run();


};
#endif //voltageDetect