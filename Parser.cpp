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


// Serial communications state machine
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
      source = SERIAL_INPUT;
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


// CAN communications state machine
void Parser::canCommunicationSM(){
  switch(canCurrentState){
    case READ:
      if (CustomCAN::getDataAvailable()){
        CustomCAN::setDataAvailable(false);
        canCurrentState = ACTUATE;
      }
      break;
      
    // case PARSE:
    //   noInterrupts();
    //   msg = CustomCAN::getcanMsgRx(); //local copy
    //   interrupts();
    //   receiverBoardNumber = msg.can_id & FILTER_BOARD_NUMBER;                      // extract the receiver
    //   senderBoardNumber = (msg.can_id >> BOARD_NUMBER_BITS) & FILTER_BOARD_NUMBER; // extract the sender
    //   responseFlag = (msg.can_id >> (2*BOARD_NUMBER_BITS)) & FILTER_MESSAGE_TYPE;   // extract the message type

    //   //Debug
    //   Serial.println("Board " + String(receiverBoardNumber) +
    //                 " received a message from board " + String(senderBoardNumber) +
    //                 " of type " + String(responseFlag));
      
    //   switch (responseFlag){
    //     case WAKE_UP:
    //       Serial.println("Entered wake up...");
    //       break;
    //     case SIMPLE_COMMAND:
    //       parseSimpleCommand(msg);
    //       break;
    //     case FLOAT_COMMAND:
    //       float value;
    //       memcpy(&value,&msg.data[1],sizeof(value));
    //       wordsCAN[0] = String(msg.data[0]);
    //       wordsCAN[1] = String(receiverBoardNumber);
    //       wordsCAN[2] = String(msg.data[0]);

    //       for (size_t i = 0; i < sizeof(wordsCAN); i++)
    //       {
    //         Serial.println("Word " + String(i) + " = " + String(wordsCAN[i]));
    //       }
          
    //       break;
    //     case ACKNOWLEDGE:
    //       Serial.println("ack"); // Acknowledge
    //       break;
    //     case SIMPLE_GET_RESPONSE:
    //       parseSimpleGetResponse(msg);
    //       break;
    //     case RESTART:
    //       break;
    //   }
    //   canCurrentState = ACTUATE;
    //   break;

    case ACTUATE:
      source = CAN_INPUT;
      actuateCommand(wordsCAN);
      canCurrentState = READ;
  }
}


// Read serial when available
void Parser::readSerialCommand(){
  float timer = micros();
  serialMessage = Serial.readStringUntil('\n'); // Read the incoming message
  serialMessage.trim(); // Remove any whitespace from the beginning and end of the message
  //count time:
  readInterval = micros()-timer;
  return;
}


// Parse the command received from serial
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


// Check if the command is d <i> <val>. If it is, check errors and
// destination, then execute it and send a response or reroute it.
bool Parser::trySetDutyCycle(String* wordsArray, can_frame msg){
  // “d <i> <val>” Set duty cycle
  if (wordsArray[0]=="d" || msg.data[0] == SET_DUTY_CYCLE){
    if (source==SERIAL_INPUT)
    {
      // Check if there are non numeric values in the <val> field
      bool valContainsNonNumeric = false;
      int comma = 0;
      for (int i = 0; i < wordsArray[2].length(); i++){
        if (!isDigit(wordsArray[2].charAt(i))) {
          if (wordsArray[2].charAt(i)=='.')
          {
            comma++;
          }
          else
          {
            valContainsNonNumeric = true;
            break;
          }
          if (comma>1)
          {
            valContainsNonNumeric = true;
            break;
          }
        }
      }
      // Check if there are non numeric values in the <i> field
      bool iContainsNonNumeric = false;
      for (int i = 0; i < wordsArray[1].length(); i++){
        if (!isDigit(wordsArray[1].charAt(i))) {
          iContainsNonNumeric = true;
          break;
        }
      }
      // Check number of words
      int numWords = 0;
      for (size_t i = 0; i < sizeof(wordsArray); i++)
      {
        if (wordsArray[i].length())
        {
          numWords++;
        }
      }
      
      // VERIFICATIONS: ERRORS, ACKNOWLEDGE or SEND VIA CAN
      if (valContainsNonNumeric)
      { // err
        Serial.println("err - Not a numeric value in <val> field.");
      }
      else if (iContainsNonNumeric)
      { // err
        Serial.println("err - Not a numeric value in <i> field.");
      }
      else if (numWords != 3)
      { // err
        Serial.println("err - Expected 3 words."); 
      }
      else if (0>wordsArray[2].toInt() || wordsArray[2].toInt()>100)
      { // err
        Serial.println("err - Not a valid duty cycle (try 0-100).");
      }
      else if(wordsArray[1].toInt()>MAX_BOARDS || wordsArray[1].toInt()<=0)
      { // err
        Serial.println("err - Not a valid board number.");
      }
      else if (wordsArray[1]==Data::getBoardNumber())
      {// Acknowledge
        Data::setDutyCycle(wordsArray[2].toFloat());
        Serial.println("ack");
      }
      else
      {// Reroute via CAN to the proper board
        //1 - Convert the float to byte array
        can_frame canMsgTx;
        float val = wordsArray[2].toFloat();
        memcpy(&canMsgTx.data[1], &val, sizeof(float));
        //2 - Fill message
        canMsgTx.can_id = (wordsArray[1].toInt() & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((RESET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = sizeof(float)+1;
        canMsgTx.data[0] = SET_DUTY_CYCLE;
        //3 - Send
        CustomCAN::SendMessage(&canMsgTx);
      }
    }
    else if (source==CAN_INPUT)
    { //If received via CAN
      receiverBoardNumber = msg.can_id & FILTER_BOARD_NUMBER;                      // extract the receiver
      senderBoardNumber = (msg.can_id >> BOARD_NUMBER_BITS) & FILTER_BOARD_NUMBER; // extract the sender
      responseFlag = (msg.can_id >> (2*BOARD_NUMBER_BITS)) & FILTER_MESSAGE_TYPE;  // extract the response flag
      
      if (responseFlag)
      { //if its the response then print to serial
        Serial.println("ack");
      }
      else
      { //if its not the response, it's a command to actuate
        //1 - get the float
        float val;
        memcpy(&val,&msg.data[1],sizeof(float));
        //2 - actuate
        Data::setDutyCycle(val);
        //3 - then send a response to acknowledge
        can_frame canMsgTx;
        canMsgTx.can_id = (senderBoardNumber & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((SET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = 1;
        canMsgTx.data[0] = SET_DUTY_CYCLE;
        CustomCAN::SendMessage(&canMsgTx);
      } 
    }  
    return true;
  }
  return false;
}


// Check if the command is g d <i>. If it is, check errors and
// destination, then execute it and send a response or reroute it.
bool Parser::tryGetDutyCycle(String* wordsArray, can_frame msg){
  // “g d <i>” Get duty cycle
  if (wordsArray[0]=="g" && wordsArray[1]=="d"|| msg.data[0] == GET_DUTY_CYCLE){
    if (source==SERIAL_INPUT)
    {
      // Check if there are non numeric values in the <i> field
      bool iContainsNonNumeric = false;
      for (int i = 0; i < wordsArray[2].length(); i++){
        if (!isDigit(wordsArray[2].charAt(i))) {
          iContainsNonNumeric = true;
          break;
        }
      }
      // Check number of words
      int numWords = 0;
      for (size_t i = 0; i < sizeof(wordsArray); i++)
      {
        if (wordsArray[i].length())
        {
          numWords++;
        }
      }
      
      // VERIFICATIONS: ERRORS, ACKNOWLEDGE or SEND VIA CAN
      if (iContainsNonNumeric)
      { // err
        Serial.println("err - Not a numeric value in <i> field.");
      }
      else if (numWords != 3)
      { // err
        Serial.println("err - Expected 3 words."); 
      }
      else if(wordsArray[2].toInt()>MAX_BOARDS || wordsArray[2].toInt()<=0)
      { // err
        Serial.println("err - Not a valid board number.");
      }
      else if (wordsArray[2]==Data::getBoardNumber())
      {// Acknowledge
        Serial.println("d " + wordsArray[2] + " " + String(Data::getDutyCycle()));  // d <i> <val>
      }
      else
      {// Reroute via CAN to the proper board
        can_frame canMsgTx;
        canMsgTx.can_id = (wordsArray[1].toInt() & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((RESET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = 1;
        canMsgTx.data[0] = GET_DUTY_CYCLE;
        //Send
        CustomCAN::SendMessage(&canMsgTx);
      }
    }
    else if (source==CAN_INPUT)
    { //If received via CAN
      receiverBoardNumber = msg.can_id & FILTER_BOARD_NUMBER;                      // extract the receiver
      senderBoardNumber = (msg.can_id >> BOARD_NUMBER_BITS) & FILTER_BOARD_NUMBER; // extract the sender
      responseFlag = (msg.can_id >> (2*BOARD_NUMBER_BITS)) & FILTER_MESSAGE_TYPE;  // extract the response flag
      
      if (responseFlag)
      { //if its the response then print to serial
        //1 - get the float
        float val;
        memcpy(&val,&msg.data[1],sizeof(float));
        //2 - print
        Serial.println("d " + String(senderBoardNumber) + " " + String(val));  // d <i> <val>
      }
      else
      { //if its not the response, it's a command to actuate
        //1 - do the get command
        float val = Data::getDutyCycle();
        Serial.println(val);
        //2 - Fill data
        can_frame canMsgTx;
        canMsgTx.can_id = (senderBoardNumber & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((SET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = sizeof(float)+1;
        canMsgTx.data[0] = GET_DUTY_CYCLE;
        memcpy(&canMsgTx.data[1],&val,sizeof(float));
        //3 - Send response message
        CustomCAN::SendMessage(&canMsgTx);
      } 
    }  
    return true;
  }
  return false;
}


// Check if the command is r <i> <val>. If it is, check errors and
// destination, then execute it and send a response or reroute it.
bool Parser::trySetReference(String* wordsArray, can_frame msg){
  //“r <i> <val>” Set duty cycle
  

  //   return true;
  // }
  return false;
}


// Checks the commands for a match
void Parser::actuateCommand(String* wordsArray){
  float timer = micros();
  can_frame msg;
  if (source==CAN_INPUT)
  {// If message is coming from CAN create local copy
    noInterrupts();
    msg = CustomCAN::getcanMsgRx(); //local copy
    interrupts();
  }
  else if(source==SERIAL_INPUT)
  {// If message is coming from serial set message identifier to none
    msg.data[0] = NONE;
  }
  

  // “d <i> <val>” Set duty cycle
  if (trySetDutyCycle(wordsArray, msg)){
    return;
  }
  
  // “g d <i>” Get duty cycle
  if (tryGetDutyCycle(wordsArray, msg)){
    return;
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
