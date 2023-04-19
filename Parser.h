#ifndef PARSER_H_
#define PARSER_H_
#include <Arduino.h>
#include "Data.h"
#include "ADC.h"
#include "can.h"

class Parser : public Data, public CustomCAN {
  public:
    typedef void (Data::*setFloat)(float);
    typedef float (Data::*getFloat)() const;
    typedef void (Data::*setBool)(bool);
    typedef bool (Data::*getBool)() const;

    Parser();
    void serialCommunicationSM(); //calls the other serial functions
    void canCommunicationSM();
    void reset() {
      serialCurrentState = READ;
      canCurrentState = READ;
      serialMessage = "";
      getSerialDuration = false;
      confirmSerialMessage = false;
      readInterval = 0;
      parseInterval = 0;
      actuationInterval = 0;
      Data::reset();
    }
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
      SET_FEEDBACK,
      GET_FEEDBACK,
      GET_LAST_MINUTE_BUFFER,
      GET_BOARD,
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
    static const int BOARD_NUMBER_BITS = 14;  // Number of board identifying bits.
    static const int FILTER_BOARD_NUMBER = 0x3FFF;  // Comparison bits for decoding can id;
    static const int FILTER_MESSAGE_TYPE = 0x1;  // Comparison bits for decoding can id;

    // Variables
    String serialMessage; //raw message
    static String wordsSerial[MAX_WORD_LENGTH]; //array of words used in serial comms
    static String wordsCAN[MAX_WORD_LENGTH]; //array of words used in CAN comms
    int receiverBoardNumber;
    int senderBoardNumber;
    int responseFlag;
    can_frame local_msg;

    // Debug flags
    bool getSerialDuration;
    bool confirmSerialMessage;
    float readInterval, parseInterval, actuationInterval; //measurement of the time it takes for each function

    //Methods
    void readSerialCommand();
    void parseSerialCommand(); //parses from a full string to an array of strings
    bool trySetCommandFloat(String* wordsArray, can_frame msg, int messageIdentifier, String commandIdentifier, setFloat func, float min, float max);
    bool tryGetCommandFloat(String* wordsArray, can_frame msg, int messageIdentifier, String commandIdentifier, getFloat func);
    bool trySetCommandBool(String* wordsArray, can_frame msg, int messageIdentifier, String commandIdentifier, setBool func);
    bool tryGetCommandBool(String* wordsArray, can_frame msg, int messageIdentifier, String commandIdentifier, getBool func);
    void tryWakeUp(can_frame msg);
    void actuateCommand(String* wordsArray); //Sets flags and executes getters
};

#endif
