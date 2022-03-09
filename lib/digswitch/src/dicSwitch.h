#include <Arduino.h>
#include "button.h"
#include "led.h"

class dicSwitch {
    private:
        bool state = false;
        Button* button;
        LED* light;
    public:
        dicSwitch(int buttonPin, int relayPin) {
            button = new Button(buttonPin, 1);
            light = new LED (relayPin);
        }
        void run()
        {
            ButtonClicks click = button->run();

            static long lastCall = 0;
            if(millis() < 20000)
            {
                if((millis() - lastCall) > 1000)
                {
                    state = !state;
                    lastCall = millis();
                }
            }
            if(click.shortClick > 0)
            {
                state = !state;
            }

            if(state)
            {
                light->ledOn();
            }
            else
            {
                light->ledOff();
            }
        }
};