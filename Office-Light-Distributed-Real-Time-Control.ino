#include "pid.h"
#include "ADC.h"
#include "Parser.h"
#include "Data.h"
#include <hardware/flash.h> //for flash_get_unique_id

// PID
pid my_pid {0.01, 0.15, 0, 0.01};
float r {25.0};

// Create a Parse object
Parser serialParser;

float pwmValue {0.0};
double lux;

// IO
const int LED_PIN = 28;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(9600);
  Serial.setTimeout(0);
}

void loop() {
  
  // Set the LED
  analogWrite(LED_PIN, r);

  serialParser.serialStateMachine();
  
  // Read voltage
  lux = getLuminance();
  
  // Controller
  float y = lux;
  float u = my_pid.compute_control(r, y);
  int pwm = (int)u;
  analogWrite(LED_PIN, pwm);
  my_pid.housekeep(r, y);
  delay(100);
  // Visualize
  /*Serial.print("Reference:");
  Serial.print(r);
  Serial.print(",");
  Serial.print("PWM:");
  Serial.print((u)/100.0);
  Serial.print(",");
  Serial.print("Lux:");
  Serial.println(lux);*/
}
