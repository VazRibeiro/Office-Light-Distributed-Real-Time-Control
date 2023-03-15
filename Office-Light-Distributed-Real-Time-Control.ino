#include "pid.h"

#define ADC_BUFFER_SIZE 100

// PID
pid my_pid {0.01, 1};
float r {25.0};

// Circular buffer
int writeIndex = 0;
float voltageArray[ADC_BUFFER_SIZE];

float pwmValue {0.0};
float voltageReading;
float resLDR;
double lux;
const int R = 10000;
const int b = 6;
float m = -0.8;

// IO
const int LED_PIN = 6;
const int SENSOR_PIN = 26;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(9600);
}


void loop() {
  // Set the LED
  /*if ( Serial.available() ){
    pwmValue = Serial.parseInt();
  }*/
  analogWrite(LED_PIN, 4095);
  // Read Sensor
  int adcReading = analogRead(SENSOR_PIN);
        // Average the sensor reading
  writeIndex > (ADC_BUFFER_SIZE-1)? writeIndex = 0 : writeIndex == writeIndex;
  voltageArray[writeIndex] = (adcReading * 3.3) / 4096;
  writeIndex++;
  float sum = 0;
  int loop = 0;
  for(loop = 0; loop < ADC_BUFFER_SIZE; loop++) {
    sum = sum + voltageArray[loop];
  }
  voltageReading = (float)sum / ADC_BUFFER_SIZE;
        // Resistance and Lux values
  resLDR = (R*3.3-voltageReading*R)/voltageReading;
  lux = pow(10,(log10(resLDR)-(float)b)/m);
  // Controller
  float y = lux;
  float u = my_pid.compute_control(r, y);
  int pwm = (int)u;
  analogWrite(LED_PIN, pwm);
  my_pid.housekeep(r, y);
  delay(10);
  // Visualize
  Serial.print("Reference:");
  Serial.print(r);
  Serial.print(",");
  Serial.print("PWM:");
  Serial.print(u);
  Serial.print(",");
  Serial.print("Lux:");
  Serial.println(lux);
}