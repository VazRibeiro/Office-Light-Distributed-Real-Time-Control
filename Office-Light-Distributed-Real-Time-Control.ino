#include "pid.h"
#include "ADC.h"
#include "Parser.h"
#include "Data.h"
#include <hardware/flash.h>
#include "mcp2515.h"

// Flash
uint8_t this_pico_flash_id[8], node_address;

// CAN
struct can_frame canMsgTx, canMsgRx;
unsigned long counterTx {0}, counterRx {0};
MCP2515::ERROR err;
unsigned long time_to_write;
unsigned long write_delay {1000};
const byte interruptPin {20};
volatile byte data_available {false};

//CAN Mask
MCP2515 can0 {spi0, 17, 19, 16, 18, 10000000};


// Class instances
pid my_pid {0.01, 50, 1, 0.5}; // h, K, b, Ti
Parser serialParser;

// flags and auxiliary variables
int counter {0};
int period {0};
bool plotJitter {false};
bool plotControl {true};
bool canSend {false};
bool canReceive {false};

// IO
const int LED_PIN {28};

// Interrupts
volatile unsigned long int timer1_time {0};
volatile bool timer1_fired {false};
struct repeating_timer timer1;
volatile unsigned long int timer2_time {0};
volatile bool timer2_fired {false};
struct repeating_timer timer2;

////////////////////////////////// CALLBACKS //////////////////////////////////
// Callback for timer 1
bool my_repeating_timer_callback1(struct repeating_timer *t )
{
  if(!timer1_fired){
    noInterrupts();
    timer1_time = micros();
    timer1_fired = true;
    interrupts();
  }
  return true;
}

// Callback for timer 2
bool my_repeating_timer_callback2(struct repeating_timer *t )
{
  if(!timer2_fired){
    noInterrupts();
    timer2_time = micros();
    timer2_fired = true;
    interrupts();
  }
  return true;
}

//the interrupt service routine for CAN
void read_interrupt(uint gpio, uint32_t events) {
  can0.readMessage(&canMsgRx);
  data_available = true;
}
////////////////////////////////// SETUP //////////////////////////////////
void setup(){
  //Identify board
  flash_get_unique_id(this_pico_flash_id);
  node_address = this_pico_flash_id[7];
  switch(node_address){ // Set board number to be able to parse serial messages
    case 39:
      serialParser.Data::setBoardNumber("1");
      break;
    case 52:
      serialParser.Data::setBoardNumber("2");
      break;
    case 33:
      serialParser.Data::setBoardNumber("3");
      break;
  }
  // Setup interrupts
  add_repeating_timer_ms( 10,my_repeating_timer_callback1,NULL, &timer1); //100 Hz interrupt
  add_repeating_timer_ms( 50,my_repeating_timer_callback2,NULL, &timer2); //20 Hz interrupt
  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(9600);
  Serial.setTimeout(0); //The serial communications does not need to wait on more info coming in for this use case.
  can0.reset();
  can0.setBitrate(CAN_1000KBPS);
  can0.setNormalMode();
  gpio_set_irq_enabled_with_callback(interruptPin, GPIO_IRQ_EDGE_FALL, true, &read_interrupt );
  time_to_write = millis() + write_delay;
}


////////////////////////////////// LOOP //////////////////////////////////
void loop() {
  // CAN communications
  if( millis() >= time_to_write ) {
    canMsgTx.can_id = node_address;
    canMsgTx.can_dlc = 8;
    unsigned long div = counterTx*10;
    for( int i = 0; i < 8; i++ )
      canMsgTx.data[7-i]='0'+((div/=10)%10);
      noInterrupts();
      err = can0.sendMessage(&canMsgTx);
      interrupts();
    if(canSend){
      Serial.print("Sending message ");
      Serial.print( counterTx );
      Serial.print(" from node ");
      Serial.println( node_address, HEX );
    }
    counterTx++;
    time_to_write = millis() + write_delay;
  }
  if( data_available ) {
    noInterrupts();
    can_frame frm {canMsgRx}; //local copy
    interrupts();
    if(canReceive){
      Serial.print("Received message number ");
      Serial.print( counterRx++ );
      Serial.print(" from node ");
      Serial.print( frm.can_id , HEX);
      Serial.print(" : ");
      for (int i=0 ; i < frm.can_dlc ; i++)
        Serial.print((char) frm.data[i]);
      Serial.println(" ");
    }
    data_available = false;
  }
  
  // Serial running at 20 Hz
  if(timer2_fired){
    serialParser.serialStateMachine(); // Serial communication state machine
    timer2_fired = false;
  }

  // Control running at 100 Hz
  if(timer1_fired){
    
    double lux;
    lux = getLuminance();// Read voltage
    period = micros()-counter;// period to calculate jitter
    counter = micros();
    
    // Controller
    float r = serialParser.Data::getReference();
    float y = lux;
    float v = my_pid.compute_control(r, y); //unsaturated output
    float u = my_pid.saturate_output(v);
    int pwm = (int)u;
    analogWrite(LED_PIN, pwm); // Set the LED
    my_pid.housekeep(r, y, v, u);

    
    // Visualization commands
    if(serialParser.Data::getIlluminanceStreamValues()){
      Serial.println("s l " + serialParser.Data::getBoardNumber() + " " + lux + " " + millis());
    }
    if(serialParser.Data::getDutyCycleStreamValues()){
      Serial.println("s d " + serialParser.Data::getBoardNumber() + " " + u/4096.0 + " " + millis());
    }
    
    // Plot controller
    if(plotJitter){
      Serial.print("Jitter:");
      Serial.println(period-10*1000);
    }
    // Plot controller
    if(plotControl){
      Serial.print("Reference:");
      Serial.print(r);
      Serial.print(",");
      Serial.print("PWM:");
      Serial.print((u)/100.0);
      Serial.print(",");
      Serial.print("Lux:");
      Serial.println(lux);
    }
    
    timer1_fired = false;
  }
}
