#include "Parser.h"


/*// state machine states
enum serialState { READ , PARSE , ACTUATE };
serialState currentState = READ;

float readInterval, parseInterval, actuationInterval = 0; //measurement of the time it takes for each function
const int MAX_MESSAGE_LENGTH = 20; // Maximum allowed message length
const int MAX_WORD_LENGTH = 4; // Maximum allowed message length
String serialMessage = ""; // raw message
String* wordsSerial; // array of words

// Debug flags
bool getSerialDuration = true;
bool confirmSerialMessage = true;*/
String Parser::wordsSerial[MAX_WORD_LENGTH] = {"","","",""};

// Constructor
Parser::Parser() 
  : currentState(READ),
    serialMessage(""), 
    getSerialDuration(true), 
    confirmSerialMessage(true), 
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

  if (wordsSerial[0]=="d"){
    Serial.println("Actuated!");
  }
  //count time:
  actuationInterval = micros()-timer;
  return;
}
