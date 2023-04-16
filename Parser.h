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
      FLOAT_COMMAND,
      SIMPLE_GET_RESPONSE,
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
    static const int BOARD_NUMBER_BITS = 12;  // Number of board identifying bits.
    static const int FILTER_BOARD_NUMBER = 0x0FFF;  // Comparison bits for decoding can id;
    static const int FILTER_MESSAGE_TYPE = 0x1F;  // Comparison bits for decoding can id;

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
    void parseSimpleGetResponse(can_frame msg);
    String redoCommand(String* wordsArray);
    bool trySetDutyCycle(String* wordsArray, String fullCommand);
    bool tryGetDutyCycle(String* wordsArray, String fullCommand);
    bool trySetReference(String* wordsArray, String fullCommand);
    void actuateCommand(String* wordsArray); //Sets flags and executes getters
};

#endif
