#include "pid.h"
#include <hardware/flash.h> //for flash_get_unique_id
#include "mcp2515.h"

#define ADC_BUFFER_SIZE 100

// CAN
uint8_t this_pico_flash_id[8], node_address;
struct can_frame canMsgTx, canMsgRx;
unsigned long counterTx {0}, counterRx {0};
MCP2515::ERROR err;
unsigned long time_to_write;
unsigned long write_delay {1000};
const byte interruptPin {20};
volatile byte data_available {false};
MCP2515 can0 {spi0, 17, 19, 16, 18, 10000000};

// PID
pid my_pid {0.01, 0.15, 0, 0.01};
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


// INTERRUPTS--------------
void read_interrupt(uint gpio, uint32_t events) {
can0.readMessage(&canMsgRx);
data_available = true;
}
// ------------------------

void setup() {
  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(9600);

  // CAN
  flash_get_unique_id(this_pico_flash_id);
  node_address = this_pico_flash_id[7];
  can0.reset();
  can0.setBitrate(CAN_1000KBPS);
  can0.setNormalMode();
  gpio_set_irq_enabled_with_callback(
    interruptPin, GPIO_IRQ_EDGE_FALL,
    true, &read_interrupt );
  time_to_write = millis() + write_delay;
}


void loop() {
  
  /*// Set the LED
  if ( Serial.available() ){
    r = Serial.parseInt(SKIP_ALL);}
  analogWrite(LED_PIN, r);
  
  // Read Sensor
  int adcReading = analogRead(SENSOR_PIN);
        // Average the sensor reading
  writeIndex > (ADC_BUFFER_SIZE-1)? writeIndex = 0 : true;
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
  lux = pow(10,(log10(resLDR)-(float)b)/m);*/

  analogWrite(LED_PIN, 500);
  // CAN
  if( millis() >= time_to_write ) {
    canMsgTx.can_id = node_address;
    canMsgTx.can_dlc = 8;
    unsigned long div = counterTx*10;
    for( int i = 0; i < 8; i++ )
      canMsgTx.data[7-i]='0'+((div/=10)%10);
      noInterrupts();
      err = can0.sendMessage(&canMsgTx);
      interrupts();
    Serial.print("Sending message ");
    Serial.print( counterTx );
    Serial.print(" from node ");
    Serial.println( node_address, HEX );
    counterTx++;
    time_to_write = millis() + write_delay;
  }
  if( data_available ) {
    noInterrupts();
    can_frame frm {canMsgRx}; //local copy
    interrupts();
    Serial.print("Received message number ");
    Serial.print( counterRx++ );
    Serial.print(" from node ");
    Serial.print( frm.can_id , HEX);
    Serial.print(" : ");
    for (int i=0 ; i < frm.can_dlc ; i++)
      Serial.print((char) frm.data[i]);
    Serial.println(" ");
    data_available = false;
  }
  // Controller
  /*float y = lux;
  float u = my_pid.compute_control(r, y);
  int pwm = (int)u;
  analogWrite(LED_PIN, pwm);
  my_pid.housekeep(r, y);
  delay(10);*/
  
  // Visualize
  /*Serial.print("Reference:");
  Serial.print(r);
  Serial.print(",");
  Serial.print("PWM:");
  Serial.print((u));
  Serial.print(",");
  Serial.print("Lux:");
  Serial.println(lux);*/
}
