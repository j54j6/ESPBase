#include <Arduino.h>
#include "button.h" //button detect

#define BUTTON_INPUT_PIN 12 //D6
#define BUTTON_LONG_CLICK_TIME 500 //Long Press = 1s
#define BUTTON_SHORT_CLICK_TIME 50 // Short Press = 0.2s
#define BUTTON_MAX_CLICK_DELAY 800 //max. Delay between Clicks 

short btnTotalPressTime = 0;
unsigned long btnStartPressTime = 0;
unsigned long btnEndPressTime = 0;
unsigned long lastPressTime = 0;
short longClick = 0;
short shortClick = 0;
int buttonPin;
short maxClicks = 1;
bool actionInit = false;

Button::Button(int buttonPin, short maxClicks)
{
    this->buttonPin = buttonPin;
    this->maxClicks = maxClicks;
    pinMode(buttonPin, INPUT);
}

void Button::resetButton()
{
    btnTotalPressTime = 0;
    longClick = 0;
    shortClick = 0;
    actionInit = false;
}

void Button::resetTime()
{
    btnTotalPressTime = 0;
    btnStartPressTime = 0;
    btnEndPressTime = 0;
}

void Button::checkButton()
{
    if(digitalRead(buttonPin) == HIGH && btnStartPressTime == 0)
    {
        btnStartPressTime = millis();
    }
    else if(digitalRead(buttonPin) == LOW && btnStartPressTime != 0)
    {
        btnEndPressTime = millis();
        lastPressTime = millis();
        btnTotalPressTime = btnEndPressTime - btnStartPressTime;
        definePressType();
    }
}

void Button::definePressType()
{
    if(btnTotalPressTime >= BUTTON_LONG_CLICK_TIME)
    {
        longClick++;
        resetTime();
    }
    else if(btnTotalPressTime >= BUTTON_SHORT_CLICK_TIME && btnTotalPressTime < BUTTON_LONG_CLICK_TIME)
    {
        shortClick++;
        resetTime();
    }
}

bool Button::checkForAction()
{
    if((digitalRead(buttonPin) == LOW && actionInit == false && ((lastPressTime != 0 && (millis() - lastPressTime) > BUTTON_MAX_CLICK_DELAY) || ((longClick + shortClick) > maxClicks))))
    {
        actionInit = true;
        return true;
    }
    return false;
}

ButtonClicks Button::run()
{
    ButtonClicks output;
    checkButton();
    bool action = checkForAction();
    if(action)
    {
        output.longClick = this->longClick;
        output.shortClick = this->shortClick;
        resetButton();
        return output;
    }
    output.longClick = 0;
    output.shortClick = 0;
    return output;
}
