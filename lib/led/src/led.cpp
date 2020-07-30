#include <Arduino.h>
#include "led.h"
/*
 int led_Pin;
 bool state = false; //false = Led off, true = Led on
 bool blinking = false;
 bool blinkInfinit = false;
 int blinkAmount = 0;
 int alreadyBlinked = 0;
 unsigned short int intervall = 0;
 bool change; //only run 
 unsigned long nextCall = 0;
 unsigned long nextToggle = 0;


*/

 bool LED::ledEnabled;

 LED::LED(){}


 LED::LED(int led_Pin, const char* ledName) //This Constructor is for debugging purposes only ! - It works also fine but the name cannot used somewhere else
 {
     ledEnabled = true;
     this->led_Pin = led_Pin;
     pinMode(led_Pin, OUTPUT);
     this->nextCall = millis();
     this->nextToggle = millis();
     this->ledName = ledName;
 }

 
 LED::LED(int led_Pin)
 {
     ledEnabled = true;
     this->led_Pin = led_Pin;
     pinMode(led_Pin, OUTPUT);
     this->nextCall = millis();
     this->nextToggle = millis();
 }
 void LED::run()
 {
    if(ledEnabled)
    {
        if(millis() > this->nextCall)
        {
            if(this->change == true || this->blinking == true)
            {
                if(!this->blinking)
                {
                    this->nextCall = millis() + LED_CALL_DELAY; 
                    this->change = false; //change only effective if blink is false
                    if(this->state)
                    {
                        this->ledOn();
                    }
                    else
                    {
                        this->ledOff();
                    }
                }
                else
                {
                    if(millis() > this->nextToggle)
                    {
                        if((this->alreadyBlinked < this->blinkAmount && this->blinkInfinit == false) || this->blinkInfinit == true)
                        {
                            if(this->intervall > 0)
                            {
                                this->nextToggle = millis() + intervall;
                                this->nextCall = nextToggle;
                                this->toggleLed(false);

                                if(this->blinkInfinit == false && this->blinking == true)
                                {
                                    alreadyBlinked++;
                                }
                            }
                        }
                        else
                        {
                            if(this->alreadyBlinked >= this->blinkAmount && this->blinking == true && this->blinkInfinit == false && this->change == false)
                            {
                                Serial.println("resetBlink");
                                this->blinking = false;
                                this->blinkAmount = 0;
                                this->alreadyBlinked = 0;
                                this->intervall = 0;
                                this->change = true;
                                this->nextCall = millis() + LED_CALL_DELAY; 
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        digitalWrite(led_Pin, LOW);
    }

 }

void LED::getVars()
{
    Serial.print("State: ");
    Serial.println(state);
    Serial.print("blinking: ");
    Serial.println(blinking);
    Serial.print("blinkInfinit: ");
    Serial.println(blinkInfinit);
    Serial.print("blinkAmount: ");
    Serial.println(blinkAmount);
    Serial.print("alreadyBlinked: ");
    Serial.println(alreadyBlinked);
    Serial.print("intervall: ");
    Serial.println(intervall);
    Serial.print("change: ");
    Serial.println(change);
    Serial.print("nextCall: ");
    Serial.println(nextCall);
    Serial.print("nextToggle: ");
    Serial.println(nextToggle);
}
 void LED::disable()
 {
     ledEnabled = false;
     change = true;
 }

 void LED::enable()
 {  
    ledEnabled = true;
    change = true;
 }

 void LED::ledOn(bool enableComplete)
 {
    if(!enableComplete)
    {
     this->state = true;
     digitalWrite(led_Pin, HIGH);
    }
    else
    {
      this->state = true;
      this->blinking = false;
      this->blinkAmount = 0;
      this->alreadyBlinked = 0;
      this->intervall = 0;
      this->change = true;
      digitalWrite(led_Pin, HIGH);
    } 
 }
 void LED::ledOff(bool disableComplete)
 {
    if(!disableComplete)
    {
     this->state = false;
     digitalWrite(led_Pin, LOW);
    }
    else
    {
      this->state = false;
      this->blinking = false;
      this->blinkAmount = 0;
      this->alreadyBlinked = 0;
      this->intervall = 0;
      this->change = true;
      digitalWrite(led_Pin, LOW);
    }
     
 }
 void LED::blink(unsigned short newIntervall, bool blinkInfinit, int amount)
 {
     this->blinkAmount = amount + amount;
     this->blinkInfinit = blinkInfinit;
     this->change = true;
     setLedBlink(newIntervall);
 }
 void LED::toggleLed(bool changeNew)
 {
     /*
     this->change = changeNew;
     if(digitalRead(led_Pin) == HIGH)
     {
         digitalWrite(led_Pin, LOW);
         state = false;
     }
     else
     {
        digitalWrite(led_Pin, HIGH);
        state = true;
     }
     */
    if(state == true)
    {
        state = false;
        ledOff(false);
    }
    else
    {
        state = true;
        ledOn(false);
    }
    change = changeNew;
 }

 void LED::setLedBlinkAmount(int amount)
 {
     this->blinkAmount = amount;
 }

 void LED::setLedState(bool state)
 {
     this->state = state;
     this->change = true;
 }
 void LED::setLedBlink(unsigned short intervall)
 {
     if(!blinking)
     {
         this->blinking = true;
     }
     this->intervall = intervall;
 }
 void LED::setLedBlink(bool blink)
 {
     this->blinking = blink;
 }
 bool LED::getLedState()
 {
     return this->state;
 }
 bool LED::getLedblink()
 {
     return this->blinking;
 }
 
 int LED::getLedIntervall()
 {
     return this->intervall;
 }

 bool LED::getLedEnabled()
 {
     return ledEnabled;
 }

 bool LED::getBlinkInfinit()
 {
     return blinkInfinit;
 }
