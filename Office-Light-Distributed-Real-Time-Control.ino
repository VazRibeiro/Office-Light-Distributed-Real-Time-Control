#include "pid.h"
#include "ADC.h"
#include "Parser.h"
#include "Data.h"
#include "Consensus.h"
#include <hardware/flash.h>

// Flash
uint8_t this_pico_flash_id[8];
uint16_t node_address;

// Class instances
pid my_pid {0.01, 50, 1, 0.5}; // h, K, b, Ti
Parser communicationParser;
Node my_node;
int board_to_calibrate = 1;

// flags and auxiliary variables
enum programState {
      WAKE_UP_MESSAGE,
      WAKE_UP_LOOP,
      CALIBRATION,
      CONTROL
    };
enum wakeUpLoopState {
      INCREMENT_ID,
      COUNT,
      SYNCHRONIZE,
      FINISHED
    };
programState mainLoopState = WAKE_UP_MESSAGE;
wakeUpLoopState wakeUpState = INCREMENT_ID;
int counterJitter {0};
int period {0};
bool plotJitter {false};
bool plotControl {false};
bool canSend {true};
bool canReceive {true};
int counterCalibration{20};
int counterWakeUp{20};
int counterID{0};
int counterCalibrationTime{0};

// IO
const int LED_PIN {28};

// Interrupts
volatile bool timer1_fired {false};
struct repeating_timer timer1;
volatile bool timer2_fired {false};
struct repeating_timer timer2;
volatile bool timer3_fired {false};
struct repeating_timer timer3;


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

// Callback for timer 3
bool my_repeating_timer_callback3(struct repeating_timer *t )
{
  if(!timer3_fired){
    noInterrupts();
    timer3_fired = true;
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
  node_address = (this_pico_flash_id[7] << 8) | (this_pico_flash_id[6] & 0xFC);
  // Setup interrupts
  add_repeating_timer_ms( 10,my_repeating_timer_callback1,NULL, &timer1); //100 Hz interrupt
  add_repeating_timer_ms( 50,my_repeating_timer_callback2,NULL, &timer2); //20 Hz interrupt
  add_repeating_timer_ms( 1,my_repeating_timer_callback3,NULL, &timer3);  //1000 Hz interrupt
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(9600);
  Serial.setTimeout(0); //The serial communications does not need to wait on more info coming in for this use case.
  communicationParser.CustomCAN::setupCAN(&read_interrupt);
}


////////////////////////////////// LOOP //////////////////////////////////
void loop() {
  // Always receiving CAN messages
  communicationParser.canCommunicationSM(); // CAN receiver state machine
  // CAN running at 1000 Hz
  // if(timer3_fired){
  //   communicationParser.canCommunicationSM(); // CAN communication state machine
  //   timer3_fired = false;
  // }

  switch (mainLoopState)
  {
  ///////////////////////////// WAKE UP
  case WAKE_UP_MESSAGE:
    delay(3000);
    //Message and reset variables only once at start
    Serial.println("Waking up...");
    communicationParser.reset();
    communicationParser.Data::setNode(node_address);
    counterCalibration=20;
    counterWakeUp=20;
    counterID=0;
    counterCalibrationTime=0;
    mainLoopState = WAKE_UP_LOOP;
    break;
  case WAKE_UP_LOOP:
    //Find others boards and signal itself
    switch (wakeUpState)
    {
    case INCREMENT_ID:
      counterWakeUp=0;
      if (16384 == counterID)
      { //reached id, send it
        communicationParser.CustomCAN::SendWakeUpMessage(node_address,0x3FFF,14,
                            communicationParser.Parser::canMessageIdentifier::WAKE_UP_PARSER);
        wakeUpState = SYNCHRONIZE;
        break;
      }
      else
      {
        counterID++;
        wakeUpState = COUNT;
        break;
      }
      break;
    case COUNT:
      counterWakeUp++;
      if (counterWakeUp==100)
      { 
        wakeUpState = INCREMENT_ID;
      }
      break;
    case SYNCHRONIZE:
      //Keep refreshing timeout
      if (timer3_fired)
      { // increment timeout counter
        communicationParser.Data::setTimeout(communicationParser.Data::getTimeout()+1);
        timer3_fired = false;
      }
      if (communicationParser.Data::getTimeout()==5000) // 5seconds
      {
        Serial.println("Timed out...");
        wakeUpState = FINISHED;
        break;
      }
      break;
    case FINISHED:
      Serial.println("Found " + String(communicationParser.Data::getTotalNumberOfBoards()) + " boards");
      //sort my board number according to id vector
      int* arr;
      int n = communicationParser.Data::getTotalNumberOfBoards();
      arr = (int*)malloc(n * sizeof(int));
      for (int i = 0; i < n; i++) {
        arr[i] = communicationParser.Data::getNode(i);
      }
      // Bubble sort
      for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j+1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
      }
      // Find my index in the sorted array of node addresses: it will be my board number
      int index;
      for (int i = 0; i < n; i++) {
        if (arr[i] == node_address) {
          index = i;
          break;
        }
      }
      //Set board number
      communicationParser.Data::setBoardNumber(String(index+1));
      int number_of_boards = communicationParser.Data::getTotalNumberOfBoards();
      // resize the vectors
      my_node.d.resize(number_of_boards);
      my_node.c.resize(number_of_boards);
      my_node.y.resize(number_of_boards);
      my_node.d_av.resize(number_of_boards);
      my_node.k.resize(number_of_boards, std::vector<double>(number_of_boards));
      my_node.L.resize(number_of_boards);
      my_node.o.resize(number_of_boards);
      std:fill(my_node.d.begin(), my_node.d.end(), 0.0);
      //Add can filters for the board number
      communicationParser.CustomCAN::setupFilters(atoi(communicationParser.Data::getBoardNumber().c_str()));
      //No longer need to know other's node addresses
      communicationParser.Data::clearNodeBuffer();
      //Move to calibration
      mainLoopState = CALIBRATION;
      break;
    }
    break;

  ///////////////////////////// CALIBRATION
  case CALIBRATION:
    Serial.println("Calibrating...");
    if (communicationParser.Data::getcalibrationFlag()==0 && communicationParser.Data::getBoardNumber()=="1")
    {
      communicationParser.Data::setcalibrationFlag(1);
      //send message with id flag-1
    }
    else if (communicationParser.Data::getBoardNumber()=="1" &&
            communicationParser.Data::getcalibrationFlag()<=communicationParser.Data::getTotalNumberOfBoards())
    {
      if (communicationParser.Data::getcalibrationAcknowledge())
      {
        if (timer2_fired)
        {
          timer2_fired = false;
          counterCalibrationTime++;
        }
        if (counterCalibration==20)//1second interval
        {
          counterCalibrationTime = 0;
          communicationParser.Data::setcalibrationAcknowledge(false);
          communicationParser.Data::incrementcalibrationFlag();
          if (communicationParser.Data::getcalibrationFlag()<=communicationParser.Data::getTotalNumberOfBoards())
          {
            /* send message */
          }
        }
      }
    }
    else if (communicationParser.Data::getcalibrationFlag()>communicationParser.Data::getTotalNumberOfBoards())
    {
      Serial.println("Ready to operate!");
      mainLoopState = CONTROL;
      break;
    }
    break;

  ///////////////////////////// CONTROL
  case CONTROL:
    // Serial running at 20 Hz
    if(timer2_fired){
      communicationParser.serialCommunicationSM(); // Serial communication state machine
      timer2_fired = false;
    }

    // Control running at 100 Hz
    if(timer1_fired){
      double lux;
      communicationParser.Data::setIlluminance((float)getLuminance());
      lux = communicationParser.Data::getIllumminance();// Read voltage
      period = micros()-counterJitter;// period to calculate jitter
      counterJitter = micros();
      
      // Controller
      float r = communicationParser.Data::getReference();
      float y = lux;
      bool w = communicationParser.Data::getWindUp();
      bool f = communicationParser.Data::getFeedback();
      int pwm;
      if(f){
        float v = my_pid.compute_control(r, y); //unsaturated output
        float u = my_pid.saturate_output(v);
        pwm = (int)u;
        my_pid.housekeep(r, y, v, u, w);
      }

      if(!f){
        pwm = my_node.d_av[communicationParser.Data::getBoardNumber().toInt()];
      }

      analogWrite(LED_PIN, pwm); // Set the LED
      // float r = 0;
      // float u = 0;
      //enviar soluções do consensus: pwm de cada placa (d), custo total (somatorio custo individual*pwm individual)
      
      // Visualization commands
      if(communicationParser.Data::getIlluminanceStreamValues()){
        Serial.println("s l " + communicationParser.Data::getBoardNumber() + " " + lux + " " + millis());
      }
      if(communicationParser.Data::getDutyCycleStreamValues()){
        Serial.println("s d " + communicationParser.Data::getBoardNumber() + " " + pwm/4096.0 + " " + millis());
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
        Serial.print((pwm)/100.0);
        Serial.print(",");
        Serial.print("Lux:");
        Serial.println(lux);
      }
      timer1_fired = false;
    }

    // Restart check
    if (communicationParser.Data::getRestart())
    {
      communicationParser.Data::setRestart(false);
      Serial.println("Restarting...");
      mainLoopState = WAKE_UP_MESSAGE;
      wakeUpState = INCREMENT_ID;
    }
    break;
  }
}