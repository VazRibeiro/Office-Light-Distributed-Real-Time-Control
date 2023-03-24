#include "Parser.h"

void parseSerial(String input){
  String firstWord;
  String secondWord;
  String thirdWord;
  firstWord = input.substring(0, input.indexOf(" ")); //Read from start until the 1st space
  secondWord = input.substring(input.indexOf(" "), input.indexOf(" ")); //Read from 1st space until the 2nd space
  thirdWord = input.substring(input.indexOf(" "), input.indexOf("\n")); //Read from 2nd space until the line end
}
