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
    void setMessage2Process(bool m2p){message2Process=m2p;}
    bool getMessage2Process() const {return message2Process;}

    enum canMessageIdentifier {
      ZERO,
      RESTART,
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
      WAKE_UP_PARSER,
      CALIBRATION,
      NONE
    };
    enum filters{
      BOARD_NUMBER_BITS = 14,
      FILTER_BOARD_NUMBER = 0x3FFF,
      FILTER_MESSAGE_TYPE = 0x1
    };
    enum id29 {
      RESET_RESPONSE_FLAG,
      SET_RESPONSE_FLAG
    };

  private:
    // Enums
    enum state {
      READ,
      PARSE,
      ACTUATE
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
    // static const int BOARD_NUMBER_BITS = 14;  // Number of board identifying bits.
    // static const int FILTER_BOARD_NUMBER = 0x3FFF;  // Comparison bits for decoding can id;
    // static const int FILTER_MESSAGE_TYPE = 0x1;  // Comparison bits for decoding can id;

    // Variables
    const int LED_PIN {28};
    String serialMessage; //raw message
    static String wordsSerial[MAX_WORD_LENGTH]; //array of words used in serial comms
    static String wordsCAN[MAX_WORD_LENGTH]; //array of words used in CAN comms
    int receiverBoardNumber;
    int senderBoardNumber;
    int responseFlag;
    can_frame local_msg;
    bool message2Process{false};

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
    void tryCalibration(can_frame msg);
    bool tryRestart(String* wordsArray, can_frame msg);
    void actuateCommand(String* wordsArray); //Sets flags and executes getters
};

#endif
