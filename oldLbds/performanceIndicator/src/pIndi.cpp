#include "pIndi.h"

void performanceIndicator::internalControl()
{
    this->callPerSecond = 1000/(millis() - lastCall);
}

ulong performanceIndicator::getCallPerSecond()
{
    return this->callPerSecond; 
}