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
    enum canMessageType {
      WAKE_UP,
      SIMPLE_COMMAND,
      LONG_COMMAND,
      ACKNOWLEDGE,
      RESTART
    };
    enum inputSource {
      SERIAL_INPUT,
      CAN_INPUT
    };
    state serialCurrentState;
    state canCurrentState;
    inputSource source;

    // Constant limits
    static const int MAX_MESSAGE_LENGTH = 20; // Maximum allowed message length
    static const int MAX_WORD_LENGTH = 4; // Maximum allowed message length
    static const int MAX_BOARDS = 3; // Maximum board number

    // Communications variables
    String serialMessage; //raw message
    static String wordsSerial[MAX_WORD_LENGTH]; //array of words used in serial comms
    static String wordsCAN[MAX_WORD_LENGTH]; //array of words used in CAN comms
    int receiverBoardNumber;
    int messageNumber;
    int senderBoardNumber;

    // Debug flags
    bool getSerialDuration;
    bool confirmSerialMessage;
    float readInterval, parseInterval, actuationInterval; //measurement of the time it takes for each function

    //Methods
    void readSerialCommand();
    void parseSerialCommand(); //parses from a full string to an array of strings
    void parseSimpleCommand(can_frame msg);
    String redoCommand(String* wordsArray);
    bool trySetDutyCycle(String* wordsArray, String fullCommand);
    void actuateCommand(String* wordsArray); //Sets flags and executes getters
};

#endif
