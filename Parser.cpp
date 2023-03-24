#include "Parser.h"
#include "Data.h"

//////////////////////////// Variables ////////////////////////////

// state machine states
enum serialState { READ , PARSE , ACTUATE };
serialState currentState = READ;

float readInterval, parseInterval, actuationInterval = 0; //measurement of the time it takes for each function
const int MAX_MESSAGE_LENGTH = 20; // Maximum allowed message length
const int MAX_WORD_LENGTH = 4; // Maximum allowed message length
String serialMessage = "";
String* wordsSerial;

// Debug flags
bool getSerialDuration = true;
bool confirmSerialMessage = true;



//////////////////////////// Functions ////////////////////////////
void serialStateMachine(){
  switch(currentState)
  {
    case READ:
      if (Serial.available()) {
        serialMessage = readSerialCommand();
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
        wordsSerial = parseSerialCommand(&serialMessage); //obtain each of the words contained in the command as Strings
        currentState = ACTUATE;
      }
      break;
      
    case ACTUATE:
      actuateSerialCommand(wordsSerial);
    
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
      currentState = READ;
  }
}


String readSerialCommand(){
  float timer = micros();
  String message = Serial.readStringUntil('\n'); // Read the incoming message
  message.trim(); // Remove any whitespace from the beginning and end of the message
  readInterval = micros()-timer;
  return message;
}


String* parseSerialCommand(String *message){
  float timer = micros();
  // Split the message into words
  char messageBuf[MAX_MESSAGE_LENGTH+1]; // Assume the message has no more than MAX_MESSAGE_LENGTH + null terminator character '\0'
  message->toCharArray(messageBuf, MAX_MESSAGE_LENGTH+1); // Convert the message to a char array
  char* word = strtok(messageBuf, " "); // Split the message at space characters and get the first word
  
  // Save each word to an array
  static String words[MAX_WORD_LENGTH]; // Assume the message has no more than MAX_WORD_LENGTH words
  int i = 0;
  while (word != NULL && i < MAX_WORD_LENGTH) {
    words[i] = String(word);
    word = strtok(NULL, " "); // Get the next word
    i++;
  }
  parseInterval = micros()-timer;
  return words;
}


void actuateSerialCommand(String* words){
  float timer = micros();

  if (words[0]=="d"){
    Serial.println("Actuated!");
  }
  
  actuationInterval = micros()-timer;
  return;
}
