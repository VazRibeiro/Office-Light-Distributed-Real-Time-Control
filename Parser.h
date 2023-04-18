#ifndef PARSER_H_
#define PARSER_H_
#include <Arduino.h>
#include "Data.h"
#include "ADC.h"
#include "can.h"

class Parser : public Data, public CustomCAN {
  public:
    Parser();
    void serialCommunicationSM(); //calls the other serial functions
    void canCommunicationSM();
  
  private:
    // Enums
    enum state {
      READ,
      PARSE,
      ACTUATE
    };
    enum canMessageIdentifier {
      NONE,
      SET_DUTY_CYCLE,
      GET_DUTY_CYCLE,
      SET_LUX_REFERENCE,
      GET_LUX_REFERENCE,
      GET_LUX_MEASUREMENT,
      SET_OCCUPANCY,
      GET_OCCUPANCY,
      SET_ANTI_WINDUP,
      GET_ANTI_WINDUP,
      GET_LAST_MINUTE_BUFFER,
      RESTART
    };
    enum inputSource {
      SERIAL_INPUT,
      CAN_INPUT
    };
    enum id29 {
      RESET_RESPONSE_FLAG,
      SET_RESPONSE_FLAG
    };
    state serialCurrentState;
    state canCurrentState;
    inputSource source;

    // Constant limits
    static const int MAX_MESSAGE_LENGTH = 20; // Maximum allowed message length
    static const int MAX_WORD_LENGTH = 4; // Maximum allowed message length
    static const int MAX_BOARDS = 3; // Maximum board number
    static const int BOARD_NUMBER_BITS = 14;  // Number of board identifying bits.
    static const int FILTER_BOARD_NUMBER = 0x3FFF;  // Comparison bits for decoding can id;
    static const int FILTER_MESSAGE_TYPE = 0x1;  // Comparison bits for decoding can id;

    // Communications variables
    String serialMessage; //raw message
    static String wordsSerial[MAX_WORD_LENGTH]; //array of words used in serial comms
    static String wordsCAN[MAX_WORD_LENGTH]; //array of words used in CAN comms
    int receiverBoardNumber;
    int responseFlag;
    int senderBoardNumber;

    // Debug flags
    bool getSerialDuration;
    bool confirmSerialMessage;
    float readInterval, parseInterval, actuationInterval; //measurement of the time it takes for each function

    //Methods
    void readSerialCommand();
    void parseSerialCommand(); //parses from a full string to an array of strings
    bool trySetDutyCycle(String* wordsArray, can_frame msg);
    bool tryGetDutyCycle(String* wordsArray, can_frame msg);
    bool trySetReference(String* wordsArray, can_frame msg);
    void actuateCommand(String* wordsArray); //Sets flags and executes getters
};

#endif
