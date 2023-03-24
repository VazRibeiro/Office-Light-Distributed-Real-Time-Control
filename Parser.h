#ifndef PARSER_H_
#define PARSER_H_
#include <Arduino.h>
#include <vector>

String readSerialCommand();
String* parseSerialCommand(String *message); //parses from a full string to an array of strings
void actuateSerialCommand(String* words); //Sets flags and executes getters

void serialStateMachine(); //calls the other serial functions

#endif
