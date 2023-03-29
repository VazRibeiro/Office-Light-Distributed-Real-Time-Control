#include "Parser.h"

String Parser::wordsSerial[MAX_WORD_LENGTH] = {"","","",""};

// Constructor
Parser::Parser() 
  : currentState(READ),
    serialMessage(""), 
    getSerialDuration(false), 
    confirmSerialMessage(false), 
    readInterval(0), 
    parseInterval(0), 
    actuationInterval(0)
{ }


void Parser::serialStateMachine(){
  switch(currentState)
  {
    case READ:
      if (Serial.available()) {
        readSerialCommand();
        currentState = PARSE;
      }
      break;
      
    case PARSE:
      if (serialMessage.length() > MAX_MESSAGE_LENGTH) { // Check message length
        Serial.println("Error: Message too long"); // Print an error message if the message is too long
        currentState = READ;
      }
      else
      {
        parseSerialCommand(); //obtain each of the words contained in the command as Strings
        currentState = ACTUATE;
      }
      break;
      
    case ACTUATE:
      actuateSerialCommand();
    
      // write the command back to the user for debug
      if (confirmSerialMessage){
        // Print the words to the serial monitor
        Serial.println("Received the command:");
        for (int j = 0; j < MAX_WORD_LENGTH; j++) {
          Serial.print("Word ");
          Serial.print(j + 1);
          Serial.print(": ");
          Serial.println(wordsSerial[j]);
        }
      }
      // write the durations for debug
      if (getSerialDuration){
        Serial.print("Time to read:");
        Serial.print(readInterval);
        Serial.print(",");
        Serial.print("Time to parse:");
        Serial.print(parseInterval);
        Serial.print(",");
        Serial.print("Time to actuate:");
        Serial.println(actuationInterval);
      }

      for (int i = 0; i < MAX_WORD_LENGTH; i++) {
        wordsSerial[i] = "";
      }
      currentState = READ;
  }
}


void Parser::readSerialCommand(){
  float timer = micros();
  serialMessage = Serial.readStringUntil('\n'); // Read the incoming message
  serialMessage.trim(); // Remove any whitespace from the beginning and end of the message
  //count time:
  readInterval = micros()-timer;
  return;
}


void Parser::parseSerialCommand(){
  float timer = micros();
  // Split the message into words
  char messageBuf[MAX_MESSAGE_LENGTH+1]; // Assume the message has no more than MAX_MESSAGE_LENGTH + null terminator character '\0'
  serialMessage.toCharArray(messageBuf, MAX_MESSAGE_LENGTH+1); // Convert the message to a char array
  char* word = strtok(messageBuf, " "); // Split the message at space characters and get the first word
  
  // Save each word to an array
  int i = 0;
  while (word != NULL && i < MAX_WORD_LENGTH) {
    wordsSerial[i] = String(word);
    word = strtok(NULL, " "); // Get the next word
    i++;
  }
  parseInterval = micros()-timer;
  return;
}


void Parser::actuateSerialCommand(){
  float timer = micros();
  
  // “d <i> <val>” Set duty cycle
  if (wordsSerial[0]=="d" && wordsSerial[1]==Data::getBoardNumber()){
    if (0<=wordsSerial[2].toInt() && wordsSerial[2].toInt()<=100){
      Data::setDutyCycle(wordsSerial[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }
  
  // “gd <i>” Set duty cycle
  if (wordsSerial[0]=="gd" && wordsSerial[1]==Data::getBoardNumber()){
    Serial.println("d " + wordsSerial[1] + " " + Data::getDutyCycle());
  }
  
  // “r <i> <val>” Set illuminance reference
  if (wordsSerial[0]=="r" && wordsSerial[1]==Data::getBoardNumber()){
    if (0<=wordsSerial[2].toInt()){
      Data::setReference(wordsSerial[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }
  
  // “gr <i>” Get illuminance reference
  if (wordsSerial[0]=="gr" && wordsSerial[1]==Data::getBoardNumber()){
    Serial.println("r " + wordsSerial[1] + " " + Data::getReference());
  }

  // “gl <i>” Get illuminance measured
  if (wordsSerial[0]=="gl" && wordsSerial[1]==Data::getBoardNumber()){
    Serial.println("l " + wordsSerial[1] + " " + getLuminance());
  }

  // “o <i> <val>” Set occupancy state
  if (wordsSerial[0]=="o" && wordsSerial[1]==Data::getBoardNumber()){
    if (0==wordsSerial[2].toInt() || 1==wordsSerial[2].toInt()) {
      Data::setOccupancy(wordsSerial[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }

  // “go <i>” Get occupancy state
  if (wordsSerial[0]=="go" && wordsSerial[1]==Data::getBoardNumber()){
    Serial.println("o " + wordsSerial[1] + " o" + Data::getOccupancy());
  }

  // “a <i> <val>” Set anti-windup state
  if (wordsSerial[0]=="a" && wordsSerial[1]==Data::getBoardNumber()){
    if (0==wordsSerial[2].toInt() || 1==wordsSerial[2].toInt()) {
      Data::setWindUp(wordsSerial[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }

  // “ga <i>” Get anti-windup state
  if (wordsSerial[0]=="ga" && wordsSerial[1]==Data::getBoardNumber()){
    Serial.println("a " + wordsSerial[1] + " " + Data::getWindUp());
  }

  // “k <i> <val>” Set feedback state
  if (wordsSerial[0]=="k" && wordsSerial[1]==Data::getBoardNumber()){
    if (0==wordsSerial[2].toInt() || 1==wordsSerial[2].toInt()) {
      Data::setFeedback(wordsSerial[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }

  // “gk <i>” Get feedback state
  if (wordsSerial[0]=="gk" && wordsSerial[1]==Data::getBoardNumber()){
    Serial.println("k " + wordsSerial[1] + " " + Data::getFeedback());
  }

  // “s <x> <i>” Start stream of real-time variable
  if (wordsSerial[0]=="s" && wordsSerial[2]==Data::getBoardNumber()){
    if ("l"==wordsSerial[1]) {
      Data::setIlluminanceStreamValues(true);
    }
    if ("d"==wordsSerial[1]) {
      Data::setDutyCycleStreamValues(true);
    }
  }

  // “S <x> <i>” Stop stream of real-time variable
  if (wordsSerial[0]=="S" && wordsSerial[2]==Data::getBoardNumber()){
    if ("l"==wordsSerial[1]) {
      Data::setIlluminanceStreamValues(false);
      Serial.println("ack");
    }
    else if ("d"==wordsSerial[1]) {
      Data::setDutyCycleStreamValues(false);
      Serial.println("ack");
    }
    else{
      Serial.println("err");
    }
  }
  
  //count time:
  actuationInterval = micros()-timer;
  return;
}
