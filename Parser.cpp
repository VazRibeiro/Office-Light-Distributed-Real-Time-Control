#include "Parser.h"

float readInterval, parseInterval = 0; //measurement of the time it takes for each function
const int MAX_MESSAGE_LENGTH = 20; // Maximum allowed message length
const int MAX_WORD_LENGTH = 4; // Maximum allowed message length
bool getSerialDuration = true;
bool confirmSerialMessage = true;

String readSerial(){
  float timer = micros();
  String message = Serial.readStringUntil('\n'); // Read the incoming message
  message.trim(); // Remove any whitespace from the beginning and end of the message
  readInterval = micros()-timer;
  return message;
}

String* parseSerial(String *message){
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

void getSerialCommand(){

  if (Serial.available()) {
    String message = readSerial();
    
    if (message.length() > MAX_MESSAGE_LENGTH) { // Check message length
      Serial.println("Error: Message too long"); // Print an error message if the message is too long
      return; // Exit the loop without processing the message
    }

    String* words;
    words = parseSerial(&message); //obtain each of the words contained in the command as Strings

    // write the command back to the user for debug
    if (confirmSerialMessage){
      // Print the words to the serial monitor
      Serial.println("Received the command:");
      for (int j = 0; j < MAX_WORD_LENGTH; j++) {
        Serial.print("Word ");
        Serial.print(j + 1);
        Serial.print(": ");
        Serial.println(words[j]);
      }
    }
    // write the durations for debug
    if (getSerialDuration){
      Serial.print("Time to read:");
      Serial.print(readInterval);
      Serial.print(",");
      Serial.print("Time to parse:");
      Serial.println(parseInterval);
    }
  }
}
