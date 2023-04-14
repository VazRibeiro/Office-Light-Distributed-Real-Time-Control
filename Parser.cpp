#include "Parser.h"

String Parser::wordsSerial[MAX_WORD_LENGTH] = {"","","",""};
String Parser::wordsCAN[MAX_WORD_LENGTH] = {"","","",""};

// Constructor
Parser::Parser() 
  : serialCurrentState(READ),
    canCurrentState(READ),
    serialMessage(""), 
    getSerialDuration(false),
    confirmSerialMessage(false),
    readInterval(0), 
    parseInterval(0), 
    actuationInterval(0)
{ }


void Parser::serialCommunicationSM(){
  switch(serialCurrentState)
  {
    case READ:
      if (Serial.available()) {
        readSerialCommand();
        serialCurrentState = PARSE;
      }
      break;
      
    case PARSE:
      if (serialMessage.length() > MAX_MESSAGE_LENGTH) { // Check message length
        Serial.println("Error: Message too long"); // Print an error message if the message is too long
        serialCurrentState = READ;
      }
      else
      {
        parseSerialCommand(); //obtain each of the words contained in the command as Strings
        serialCurrentState = ACTUATE;
      }
      break;
    
    case ACTUATE:
      actuateCommand(wordsSerial);
    
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
      serialCurrentState = READ;
  }
}


void Parser::canCommunicationSM(){
  int messageNumber = 0;
  int senderBoardNumber = 0;
  can_frame msg;
  switch(canCurrentState)
  {
    case READ:
      if (CustomCAN::getDataAvailable()){
        CustomCAN::setDataAvailable(false);
        canCurrentState = PARSE;
      }
      break;
      
    case PARSE:
      noInterrupts();
      msg = CustomCAN::getcanMsgRx(); //local copy
      interrupts();
      
      senderBoardNumber = (msg.can_id >> 4) & 0xF; // extract the sender
      messageNumber = msg.can_id >> 8;    // shift the bits to the right by 4 to get the remaining bits
      //Debug
      Serial.println("Board " + Data::getBoardNumber() + " received a message from board " + String(senderBoardNumber));

      if (messageNumber == SIMPLE_COMMAND)
      {
        // Convert the data field to a char array
        char charData[msg.can_dlc + 1];
        for (int i = 0; i < msg.can_dlc; i++) {
            charData[i] = (char) msg.data[i];
        }
        charData[msg.can_dlc] = '\0'; // Null-terminate the char array

        char* word = strtok(charData, " "); // Split the message at space characters and get the first word
        
        // Save each word to an array
        int i = 0;
        while (word != NULL && i < MAX_WORD_LENGTH) {
          wordsCAN[i] = String(word);
          Serial.println("Word " + String(i) + " = " + wordsCAN[i]); // Debug
          word = strtok(NULL, " "); // Get the next word
          i++;
        }
        
        canCurrentState = ACTUATE;
      }

      break;

    case ACTUATE:
      actuateCommand(wordsCAN);

      for (int i = 0; i < MAX_WORD_LENGTH; i++) {
        wordsCAN[i] = "";
      }
      canCurrentState = READ;
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


void Parser::actuateCommand(String* wordsArray){
  float timer = micros();
  can_frame canMsgTx;
  // join words into a single full word and create a char array
  // to send all the characteres in canMsgTx.data with no spaces
  String fullCommand = "";
  for (int i = 0; i<sizeof(wordsArray);i++){ 
    fullCommand.concat(wordsArray[0]);
    if (i < 2) {
    fullCommand.concat(" "); // add space between words
    }
  }
  char charArray[fullCommand.length()+1];
  fullCommand.toCharArray(charArray, sizeof(charArray));

  
  // “d <i> <val>” Set duty cycle
  if (wordsArray[0]=="d"){
    if (wordsArray[1]==Data::getBoardNumber())
    {
      if (0<=wordsArray[2].toInt() && wordsArray[2].toInt()<=100){
        Data::setDutyCycle(wordsArray[2].toInt());
        Serial.println("ack");
      }
      else
      {
        Serial.println("err"); // Print an error message
      }
    }
    else
    {
      canMsgTx.can_id = (wordsArray[1].toInt() & 0x0F) | ((Data::getBoardNumber().toInt() & 0x0F) << 4) | ((SIMPLE_COMMAND & 0x3FF) << 8);
      canMsgTx.can_dlc = fullCommand.length();
      Serial.println("DLC = " + String(canMsgTx.can_dlc)); // Debug
      for (int i = 0; i<fullCommand.length(); i++){
        canMsgTx.data[i]=fullCommand.charAt(i);
      }
      CustomCAN::SendMessage(&canMsgTx);
      Serial.println("sending can id " + String(canMsgTx.can_id));
    }
     
  }
  
  // “gd <i>” Set duty cycle
  if (wordsArray[0]=="gd" && wordsArray[1]==Data::getBoardNumber()){
    Serial.println("d " + wordsArray[1] + " " + Data::getDutyCycle());
  }
  
  // “r <i> <val>” Set illuminance reference
  if (wordsArray[0]=="r" && wordsArray[1]==Data::getBoardNumber()){
    if (0<=wordsArray[2].toInt()){
      Data::setReference(wordsArray[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }
  
  // “gr <i>” Get illuminance reference
  if (wordsArray[0]=="gr" && wordsArray[1]==Data::getBoardNumber()){
    Serial.println("r " + wordsArray[1] + " " + Data::getReference());
  }

  // “gl <i>” Get illuminance measured
  if (wordsArray[0]=="gl" && wordsArray[1]==Data::getBoardNumber()){
    Serial.println("l " + wordsArray[1] + " " + getLuminance());
  }

  // “o <i> <val>” Set occupancy state
  if (wordsArray[0]=="o" && wordsArray[1]==Data::getBoardNumber()){
    if (0==wordsArray[2].toInt() || 1==wordsArray[2].toInt()) {
      Data::setOccupancy(wordsArray[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }

  // “go <i>” Get occupancy state
  if (wordsArray[0]=="go" && wordsArray[1]==Data::getBoardNumber()){
    Serial.println("o " + wordsArray[1] + " o" + Data::getOccupancy());
  }

  // “a <i> <val>” Set anti-windup state
  if (wordsArray[0]=="a" && wordsArray[1]==Data::getBoardNumber()){
    if (0==wordsArray[2].toInt() || 1==wordsArray[2].toInt()) {
      Data::setWindUp(wordsArray[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }

  // “ga <i>” Get anti-windup state
  if (wordsArray[0]=="ga" && wordsArray[1]==Data::getBoardNumber()){
    Serial.println("a " + wordsArray[1] + " " + Data::getWindUp());
  }

  // “k <i> <val>” Set feedback state
  if (wordsArray[0]=="k" && wordsArray[1]==Data::getBoardNumber()){
    if (0==wordsArray[2].toInt() || 1==wordsArray[2].toInt()) {
      Data::setFeedback(wordsArray[2].toInt());
      Serial.println("ack");
    }
    else
    {
      Serial.println("err"); // Print an error message
    }
  }

  // “gk <i>” Get feedback state
  if (wordsArray[0]=="gk" && wordsArray[1]==Data::getBoardNumber()){
    Serial.println("k " + wordsArray[1] + " " + Data::getFeedback());
  }

  // “s <x> <i>” Start stream of real-time variable
  if (wordsArray[0]=="s" && wordsArray[2]==Data::getBoardNumber()){
    if ("l"==wordsArray[1]) {
      Data::setIlluminanceStreamValues(true);
    }
    if ("d"==wordsArray[1]) {
      Data::setDutyCycleStreamValues(true);
    }
  }

  // “S <x> <i>” Stop stream of real-time variable
  if (wordsArray[0]=="S" && wordsArray[2]==Data::getBoardNumber()){
    if ("l"==wordsArray[1]) {
      Data::setIlluminanceStreamValues(false);
      Serial.println("ack");
    }
    else if ("d"==wordsArray[1]) {
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
