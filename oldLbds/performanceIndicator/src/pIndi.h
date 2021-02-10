#ifndef J54J6_PERFORMANCE_INDICATOR_H
#define J54J6_PERFORMANCE_INDICATOR_H
#include <Arduino.h>

class performanceIndicator {
    protected:
        ulong lastCall = 0;
        ulong callPerSecond = 0;
        void internalControl();

    public:
        ulong getCallPerSecond();
};
#endif