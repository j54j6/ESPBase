#include "lib_General.h"

bool factoryReset()
{
    if(!Serial)
    {
        Serial.begin(115200);
    }
    Serial.println("Factory Reset()!");

    return true;
}