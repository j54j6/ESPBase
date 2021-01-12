#ifndef Lib_Led_h
#define Lib_Led_h

#include <Arduino.h>
#include "logging.h"

#define LED_CALL_DELAY 50 // Led Update Time (time until changes will visible

class LED {
    private:
        const char* className = "LED";
        int led_Pin;
        bool state = false;
        bool blinking = false;
        bool blinkInfinit = false;
        int blinkAmount = 0;
        int alreadyBlinked = 0;
        unsigned short int intervall = 0;
        const char* ledName = "not set";

        bool change; //only run 

        unsigned long nextCall;
        unsigned long nextToggle = 0;

    public:
        //Led Handler Constructor struct
        struct ledConfig {
            int led_Pin;
            bool state;
        };

        static bool ledEnabled; 
        LED();
        LED(int led_Pin, const char* ledName);
        LED(int led_Pin);
        void getVars();
        void run();
        void ledOn(bool enableComplete = true);
        void ledOff(bool disableComplete = true);
        void blink(unsigned short newIntervall, bool blinkInfinit = true, int amount = 0);
        void toggleLed(bool changeNew = true);
        void setLedState(bool state = false);
        void setLedBlink(unsigned short intevall = 0);
        void setLedBlink(bool blink);
        void setLedBlinkAmount(int amount);
        bool getLedState();
        bool getLedblink();
        bool getBlinkInfinit();
        int getLedIntervall();
        void disable();
        void enable();
        bool getLedEnabled();
};
#endif