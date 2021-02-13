#include "voltageDetect.h"

voltageDetector::voltageDetector(int voltagePin, int batteryType, float powerFactor)
{
    this->voltagePin = voltagePin;
    this->factor = powerFactor;
    this->batteryType = batteryType;
    pinMode(voltagePin, INPUT);
}

float voltageDetector::getActualVoltage()
{
    return actualValue * factor;
}

float voltageDetector::getActualRawValue()
{
    return this->actualValue;
}

int voltageDetector::calcPercentage18650Battery()
{
    float voltage = getActualVoltage();
    float fVoltageMatrix[22][2] = {
        {4.15,  100},
        {4.11, 95},
        {4.08, 90},
        {4.02, 85},
        {3.98, 80},
        {3.95, 75},
        {3.91, 70},
        {3.87, 65},
        {3.85, 60},
        {3.84, 55},
        {3.82, 50},
        {3.80, 45},
        {3.79, 40},
        {3.77, 35},
        {3.75, 30},
        {3.73, 25},
        {3.71, 20},
        {3.69, 15},
        {3.31, 10},
        {3.15, 5},
        {3.00, 0},
        {0, 0}
    };

  int i, perc;

  perc = 100;

  for(i=20; i>=0; i--) {
    if(fVoltageMatrix[i][0] >= voltage) {
      perc = fVoltageMatrix[i + 1][1];
      break;
    }
  }
  return perc;
}

int voltageDetector::getPercent()
{
    switch (this->batteryType)
    {
    case 0:
        return calcPercentage18650Battery();
        break;
    
    default:
        return calcPercentage18650Battery();
    }
}

void voltageDetector::run()
{
    this->actualValue = analogRead(voltagePin);
}
