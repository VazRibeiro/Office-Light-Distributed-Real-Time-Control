#ifndef PARSER_H_
#define PARSER_H_
#include <Arduino.h>
#include <vector>

String readSerial();
String * parseSerial(String *message); //parses from a full string to an array of strings

void getSerialCommand(); //calls the other serial functions

#endif
