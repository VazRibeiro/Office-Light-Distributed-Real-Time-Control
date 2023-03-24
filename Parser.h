#ifndef PARSER_H_
#define PARSER_H_
#include <Arduino.h>
#include "Data.h"

class Parser : public Data {
  public:
    Parser();
    void readSerialCommand();
    void parseSerialCommand(); //parses from a full string to an array of strings
    void actuateSerialCommand(); //Sets flags and executes getters
    void serialStateMachine(); //calls the other serial functions
  
  private:
    enum serialState { READ , PARSE , ACTUATE };
    serialState currentState;
    static const int MAX_MESSAGE_LENGTH = 20; // Maximum allowed message length
    static const int MAX_WORD_LENGTH = 4; // Maximum allowed message length
    String serialMessage; //raw message
    static String wordsSerial[MAX_WORD_LENGTH]; //array of words
    // Debug flags
    bool getSerialDuration;
    bool confirmSerialMessage;
    float readInterval, parseInterval, actuationInterval; //measurement of the time it takes for each function
};

#endif
