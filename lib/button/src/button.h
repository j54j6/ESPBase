#pragma once
#ifndef Lib_Button_h
#define Lib_Button_h

#include "Arduino.h"

#define BUTTON_INPUT_PIN 12 //D6
#define BUTTON_LONG_CLICK_TIME 500 //Long Press = 1s
#define BUTTON_SHORT_CLICK_TIME 50 // Short Press = 0.2s
#define BUTTON_MAX_CLICK_DELAY 800 //max. Delay between Clicks 

struct ButtonClicks {
    short longClick = 0;
    short shortClick = 0;
};


class Button {
    friend class SimpleWiFiManager;
    private:
        short btnTotalPressTime = 0;
        unsigned long btnStartPressTime = 0;
        unsigned long btnEndPressTime = 0;
        unsigned long lastPressTime = 0;
        short longClick = 0;
        short shortClick = 0;
        int buttonPin;
        short maxClicks = 1;
        bool actionInit = false;
        

    public:
        Button(int buttonPin, short maxClicks);
        void resetButton();
        void resetTime();
        void checkButton();
        void definePressType();
        bool checkForAction();
        ButtonClicks run();
};

#endif