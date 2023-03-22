#include "ADC.h"

const int SENSOR_PIN = 26;
int writeIndexADC = 0;
float voltageArray[ADC_BUFFER_SIZE];
float voltageReading;
float resLDR;
const int R = 10000;
const int b = 6;
float m = -0.8;

double getLuminance(){
  // Read voltage
  int adcReading = (analogRead(SENSOR_PIN)* 3.3) / 4096;
  
  // Average the sensor reading 
  writeIndexADC > (ADC_BUFFER_SIZE-1)? writeIndexADC = 0 : true;
  voltageArray[writeIndexADC] = adcReading;
  writeIndexADC++;
  float sum = 0;
  int loop = 0;
  for(loop = 0; loop < ADC_BUFFER_SIZE; loop++) {
    sum = sum + voltageArray[loop];
  }
  voltageReading = sum / ADC_BUFFER_SIZE;
  
  // Resistance and Lux values
  resLDR = (R*3.3-voltageReading*R)/voltageReading;
  return pow(10,(log10(resLDR)-(float)b)/m);
}
