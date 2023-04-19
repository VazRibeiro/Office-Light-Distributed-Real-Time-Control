#include "pid.h"
#include "ADC.h"
#include "Parser.h"
#include "Data.h"
#include "Consensus.h"
#include <hardware/flash.h>

// Flash
uint8_t this_pico_flash_id[8], node_address;

// Class instances
pid my_pid {0.01, 50, 1, 0.5}; // h, K, b, Ti
Parser communicationParser;
Node my_node;
int number_of_boards;
int board_to_calibrate = 1;

// flags and auxiliary variables
enum programState {
      WAKE_UP,
      CALIBRATION,
      CONTROL
    };
programState mainLoopState = CONTROL;
bool deadlockLED {true};
int counter {0};
int period {0};
bool plotJitter {false};
bool plotControl {false};
bool canSend {true};
bool canReceive {true};
int counterCalibration{20};

// IO
const int LED_PIN {28};

// Interrupts
volatile bool timer1_fired {false};
struct repeating_timer timer1;
volatile bool timer2_fired {false};
struct repeating_timer timer2;


////////////////////////////////// CALLBACKS //////////////////////////////////
// Callback for timer 1
bool my_repeating_timer_callback1(struct repeating_timer *t )
{
  if(!timer1_fired){
    noInterrupts();
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
    timer2_fired = true;
    interrupts();
  }
  return true;
}

//the interrupt service routine for CAN
void read_interrupt(uint gpio, uint32_t events) {
  communicationParser.CustomCAN::ReadMessage();
  communicationParser.CustomCAN::setDataAvailable(true);
}

////////////////////////////////// SETUP //////////////////////////////////
void setup(){
  //Identify board
  flash_get_unique_id(this_pico_flash_id);
  node_address = this_pico_flash_id[7];
  switch(node_address){ // Set board number to be able to parse serial messages
    case 39:
      communicationParser.Data::setBoardNumber("1");
      break;
    case 52:
      communicationParser.Data::setBoardNumber("2");
      break;
    case 33:
      communicationParser.Data::setBoardNumber("3");
      break;
  }
  // Setup interrupts
  add_repeating_timer_ms( 10,my_repeating_timer_callback1,NULL, &timer1); //100 Hz interrupt
  add_repeating_timer_ms( 50,my_repeating_timer_callback2,NULL, &timer2); //20 Hz interrupt
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(9600);
  Serial.setTimeout(0); //The serial communications does not need to wait on more info coming in for this use case.
  communicationParser.CustomCAN::setupCAN(&read_interrupt, atoi(communicationParser.Data::getBoardNumber().c_str()));
}


////////////////////////////////// LOOP //////////////////////////////////
void loop() {
  switch (mainLoopState)
  {
  case WAKE_UP:
    number_of_boards = 3;
    // resize the vectors
    my_node.c.resize(number_of_boards);
    my_node.y.resize(number_of_boards);
    my_node.d_av.resize(number_of_boards);
    my_node.k.resize(number_of_boards, std::vector<double>(number_of_boards));
    my_node.L.resize(number_of_boards);
    my_node.o.resize(number_of_boards);
    mainLoopState = CALIBRATION;
    break;
  case CALIBRATION:
    if(timer2_fired){
      if (counterCalibration==20 && board_to_calibrate<=number_of_boards)
      {
        counterCalibration = 0;
        //mensagem para todas as placas a dizer para ligar board 1,2,...
        board_to_calibrate++;
      } else if (board_to_calibrate>number_of_boards)
      {
        mainLoopState = CONTROL;
        break;
      }
      communicationParser.canCommunicationSM(); // CAN communication state machine
      counterCalibration++;
    }
  case CONTROL:
    // Serial running at 20 Hz
    if(timer2_fired){
      if(false){
        digitalWrite(LED_BUILTIN, HIGH);
        deadlockLED = false;
      }
      else
      {
        digitalWrite(LED_BUILTIN, LOW);
        deadlockLED = true;
      }
      communicationParser.serialCommunicationSM(); // Serial communication state machine
      timer2_fired = false;
    }

    // Control running at 100 Hz
    if(timer1_fired){
      communicationParser.canCommunicationSM(); // CAN communication state machine
      double lux;
      lux = getLuminance();// Read voltage
      period = micros()-counter;// period to calculate jitter
      counter = micros();
      
      // Controller
      // float r = communicationParser.Data::getReference();
      // float y = lux;
      // float v = my_pid.compute_control(r, y); //unsaturated output
      // float u = my_pid.saturate_output(v);
      // int pwm = (int)u;
      // analogWrite(LED_PIN, pwm); // Set the LED
      // my_pid.housekeep(r, y, v, u);
      float r = 0;
      float u = 0;
      //enviar soluções do consensus: pwm de cada placa (d), custo total (somatorio custo individual*pwm individual)


      
      // Visualization commands
      if(communicationParser.Data::getIlluminanceStreamValues()){
        Serial.println("s l " + communicationParser.Data::getBoardNumber() + " " + lux + " " + millis());
      }
      if(communicationParser.Data::getDutyCycleStreamValues()){
        Serial.println("s d " + communicationParser.Data::getBoardNumber() + " " + u/4096.0 + " " + millis());
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
    break;
  }
}
