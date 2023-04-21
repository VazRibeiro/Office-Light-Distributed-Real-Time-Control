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
        noInterrupts();
        local_msg = CustomCAN::getcanMsgRx(); //local copy
        interrupts();
        canCurrentState = ACTUATE;
      }
      break;

    case ACTUATE:
      source = CAN_INPUT;
      actuateCommand(wordsCAN);
      canCurrentState = READ;
      break;
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


// Check if the command is "<commandIdentifier> <i> <val>". If it is, check
// errors and destination, then execute it and send a response or reroute it.
// This function works with <val> of type float
bool Parser::trySetCommandFloat(String* wordsArray, can_frame msg, int messageIdentifier, String commandIdentifier, setFloat func, float min, float max){
  // <commandIdentifier> <i> <val>
  if (wordsArray[0]==commandIdentifier || msg.data[0] == messageIdentifier){
    if (source==SERIAL_INPUT)
    {
      int valContainsNonNumeric = false;
      bool iContainsNonNumeric = false;
      int numWords = 0;
      int comma = 0;
      // Check if there are non numeric values in the <val> field
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
      for (int i = 0; i < wordsArray[1].length(); i++){
        if (!isDigit(wordsArray[1].charAt(i))) {
          iContainsNonNumeric = true;
          break;
        }
      }
      // Check number of words
      for (size_t i = 0; i < sizeof(wordsArray); i++)
      {
        if (wordsArray[i].length())
        {
          numWords++;
        }
      }
      // VERIFICATIONS: ERRORS, ACKNOWLEDGE or SEND VIA CAN
      if (valContainsNonNumeric)
      {
        Serial.println("err - Not a numeric value in <val> field."); // err
      }
      else if (iContainsNonNumeric)
      {
        Serial.println("err - Not a numeric value in <i> field."); // err
      }
      else if (numWords != 3)
      {
        Serial.println("err - Expected only 3 words."); // err
      }
      else if (min>wordsArray[2].toFloat() || wordsArray[2].toFloat()>max){
        Serial.println("err - Not a valid value."); // err
      }
      else if(wordsArray[1].toInt()>Data::getTotalNumberOfBoards() || wordsArray[1].toInt()<=0){
        Serial.println("err - Not a valid board number."); // err
      }
      else if (wordsArray[1]==Data::getBoardNumber())
      {// Acknowledge
        (this->*func)(wordsArray[2].toFloat());
        Serial.println("ack");
      }
      else
      {// Reroute via CAN to the proper board
        can_frame canMsgTx;
        canMsgTx.can_id = (wordsArray[1].toInt() & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((RESET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = sizeof(float)+1;
        canMsgTx.data[0] = messageIdentifier;
        float val = wordsArray[2].toFloat();
        memcpy(&canMsgTx.data[1], &val, sizeof(float));
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
        (this->*func)(val);
        //3 - then send a response to acknowledge
        can_frame canMsgTx;
        canMsgTx.can_id = (senderBoardNumber & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((SET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = 1;
        canMsgTx.data[0] = messageIdentifier;
        CustomCAN::SendMessage(&canMsgTx);
      } 
    }  
    return true;
  }
  return false;
}


// Check if the command is "<commandIdentifier> <i> <val>". If it is, check
// errors and destination, then execute it and send a response or reroute it.
// This function works with <val> of type bool
bool Parser::trySetCommandBool(String* wordsArray, can_frame msg, int messageIdentifier, String commandIdentifier, setBool func){
  // <commandIdentifier> <i> <val>
  if (wordsArray[0]==commandIdentifier || msg.data[0] == messageIdentifier){
    if (source==SERIAL_INPUT)
    {
      int valContainsNonNumeric = false;
      bool iContainsNonNumeric = false;
      int numWords = 0;
      // Check if there are non numeric values in the <val> field
      for (int i = 0; i < wordsArray[2].length(); i++){
        if (!isDigit(wordsArray[2].charAt(i))) {
          valContainsNonNumeric = true;
          break;
        }
      }
      // Check if there are non numeric values in the <i> field
      for (int i = 0; i < wordsArray[1].length(); i++){
        if (!isDigit(wordsArray[1].charAt(i))) {
          iContainsNonNumeric = true;
          break;
        }
      }
      // Check number of words
      for (size_t i = 0; i < sizeof(wordsArray); i++)
      {
        if (wordsArray[i].length())
        {
          numWords++;
        }
      }
      // VERIFICATIONS: ERRORS, ACKNOWLEDGE or SEND VIA CAN
      if (valContainsNonNumeric)
      {
        Serial.println("err - Not a numeric value in <val> field."); // err
      }
      else if (iContainsNonNumeric)
      {
        Serial.println("err - Not a numeric value in <i> field."); // err
      }
      else if (numWords != 3)
      {
        Serial.println("err - Expected only 3 words."); // err
      }
      else if (!(wordsArray[2]=="0" || wordsArray[2]=="1")){
        Serial.println("err - Not a valid value."); // err
      }
      else if(wordsArray[1].toInt()>Data::getTotalNumberOfBoards() || wordsArray[1].toInt()<=0){
        Serial.println("err - Not a valid board number."); // err
      }
      else if (wordsArray[1]==Data::getBoardNumber())
      {// Acknowledge
        if (wordsArray[2]=="0")
        {
          (this->*func)(false);
          Serial.println("ack");
        }
        else if (wordsArray[2]=="1")
        {
          (this->*func)(true);
          Serial.println("ack");
        }
      }
      else
      {// Reroute via CAN to the proper board
        can_frame canMsgTx;
        canMsgTx.can_id = (wordsArray[1].toInt() & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((RESET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = sizeof(bool)+1;
        canMsgTx.data[0] = messageIdentifier;
        bool val = wordsArray[2].toInt();
        memcpy(&canMsgTx.data[1], &val, sizeof(bool));
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
        bool val;
        memcpy(&val,&msg.data[1],sizeof(bool));
        //2 - actuate
        (this->*func)(val);
        //3 - then send a response to acknowledge
        can_frame canMsgTx;
        canMsgTx.can_id = (senderBoardNumber & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((SET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = 1;
        canMsgTx.data[0] = messageIdentifier;
        CustomCAN::SendMessage(&canMsgTx);
      } 
    }  
    return true;
  }
  return false;
}


// Check if the command is "g <commandIdentifier> <i>". If it is, check
// errors and destination, then execute it and send a response or reroute it.
// This function works with <val> of type float
bool Parser::tryGetCommandFloat(String* wordsArray, can_frame msg, int messageIdentifier, String commandIdentifier, getFloat func){
  if (wordsArray[0]=="g" && wordsArray[1]==commandIdentifier|| msg.data[0] == messageIdentifier){
    if (source==SERIAL_INPUT)
    {
      bool iContainsNonNumeric = false;
      int numWords = 0;
      // Check if there are non numeric values in the <i> field
      for (int i = 0; i < wordsArray[2].length(); i++){
        if (!isDigit(wordsArray[2].charAt(i))) {
          iContainsNonNumeric = true;
          break;
        }
      }
      // Check number of words
      for (size_t i = 0; i < sizeof(wordsArray); i++)
      {
        if (wordsArray[i].length())
        {
          numWords++;
        }
      }
      // VERIFICATIONS: ERRORS, ACKNOWLEDGE or SEND VIA CAN
      if (iContainsNonNumeric)
      {
        Serial.println("err - Not a numeric value in <i> field."); // err
      }
      else if (numWords != 3)
      {
        Serial.println("err - Expected only 3 words."); // err
      }
      else if(wordsArray[2].toInt()>Data::getTotalNumberOfBoards() || wordsArray[2].toInt()<=0){
        Serial.println("err - Not a valid board number."); // err
      }
      else if (wordsArray[2]==Data::getBoardNumber())
      {// <commandIdentifier> <i> <val>
        Serial.println(commandIdentifier + " " + wordsArray[2] + " " + String((this->*(func))()));
      }
      else
      {// Reroute via CAN to the proper board
        can_frame canMsgTx;
        canMsgTx.can_id = (wordsArray[2].toInt() & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((RESET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = 1;
        canMsgTx.data[0] = messageIdentifier;
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
        //2 - print <command identifier> <i> <val>
        Serial.println(commandIdentifier + " " + String(senderBoardNumber) + " " + String(val));  
      }
      else
      { //if its not the response, it's a command to actuate
        //1 - do the get command
        float val = (this->*(func))();
        //2 - Fill data
        can_frame canMsgTx;
        canMsgTx.can_id = (senderBoardNumber & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((SET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = sizeof(float)+1;
        canMsgTx.data[0] = messageIdentifier;
        memcpy(&canMsgTx.data[1],&val,sizeof(float));
        //3 - Send response message
        CustomCAN::SendMessage(&canMsgTx);
      } 
    }  
    return true;
  }
  return false;
}


// Check if the command is "g <commandIdentifier> <i>". If it is, check
// errors and destination, then execute it and send a response or reroute it.
// This function works with <val> of type bool
bool Parser::tryGetCommandBool(String* wordsArray, can_frame msg, int messageIdentifier, String commandIdentifier, getBool func){
  if (wordsArray[0]=="g" && wordsArray[1]==commandIdentifier || msg.data[0] == messageIdentifier){
    if (source==SERIAL_INPUT)
    {
      bool iContainsNonNumeric = false;
      int numWords = 0;
      // Check if there are non numeric values in the <i> field
      for (int i = 0; i < wordsArray[2].length(); i++){
        if (!isDigit(wordsArray[2].charAt(i))) {
          iContainsNonNumeric = true;
          break;
        }
      }
      // Check number of words
      for (size_t i = 0; i < sizeof(wordsArray); i++)
      {
        if (wordsArray[i].length())
        {
          numWords++;
        }
      }
      // VERIFICATIONS: ERRORS, ACKNOWLEDGE or SEND VIA CAN
      if (iContainsNonNumeric)
      {
        Serial.println("err - Not a numeric value in <i> field."); // err
      }
      else if (numWords != 3)
      {
        Serial.println("err - Expected only 3 words."); // err
      }
      else if(wordsArray[2].toInt()>Data::getTotalNumberOfBoards() || wordsArray[2].toInt()<=0){
        Serial.println("err - Not a valid board number."); // err
      }
      else if (wordsArray[2]==Data::getBoardNumber())
      {// <commandIdentifier> <i> <val>
        Serial.println(commandIdentifier + " " + wordsArray[2] + " " + String((this->*(func))()));
      }
      else
      {// Reroute via CAN to the proper board
        can_frame canMsgTx;
        canMsgTx.can_id = (wordsArray[2].toInt() & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((RESET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = 1;
        canMsgTx.data[0] = messageIdentifier;
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
        //1 - get the bool
        bool val;
        memcpy(&val,&msg.data[1],sizeof(bool));
        //2 - print <command identifier> <i> <val>
        Serial.println(commandIdentifier + " " + String(senderBoardNumber) + " " + String(val));  
      }
      else
      { //if its not the response, it's a command to actuate
        //1 - do the get command
        bool val = (this->*(func))();
        //2 - Fill data
        can_frame canMsgTx;
        canMsgTx.can_id = (senderBoardNumber & FILTER_BOARD_NUMBER) |
                          ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                          ((SET_RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) |
                          CAN_EFF_FLAG;
        canMsgTx.can_dlc = sizeof(bool)+1;
        canMsgTx.data[0] = messageIdentifier;
        memcpy(&canMsgTx.data[1],&val,sizeof(bool));
        //3 - Send response message
        CustomCAN::SendMessage(&canMsgTx);
      } 
    }  
    return true;
  }
  return false;
}

// Sends wake up message to other boards to introduce your node_id
void Parser::tryWakeUp(can_frame msg){
  if (source==CAN_INPUT  && msg.data[0] == WAKE_UP_PARSER)
  { //If received via CAN
    receiverBoardNumber = msg.can_id & FILTER_BOARD_NUMBER;                      // extract the receiver
    senderBoardNumber = (msg.can_id >> BOARD_NUMBER_BITS) & FILTER_BOARD_NUMBER; // extract the sender
    if (receiverBoardNumber==0 && msg.can_dlc==1)
    {
      Data::setNode(senderBoardNumber);
      Data::setTimeout(0);
    }
  }
}

// Restart
bool Parser::tryRestart(String* wordsArray, can_frame msg){
  int numWords = 0;
  // Check number of words
  for (size_t i = 0; i < sizeof(wordsArray); i++)
  {
    if (wordsArray[i].length())
    {
      numWords++;
    }
  }
  if (source==SERIAL_INPUT && wordsArray[0] == "r" && numWords == 1)
  {
    can_frame canMsgTx;
    canMsgTx.can_id = (0 & FILTER_BOARD_NUMBER) |
                      ((Data::getBoardNumber().toInt() & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                      CAN_EFF_FLAG;
    canMsgTx.can_dlc = 1;
    canMsgTx.data[0] = RESTART;
    CustomCAN::SendMessage(&canMsgTx);
    Data::setRestart(true);
    Serial.println("Restarting");
    return true;
  }
  else if (source==CAN_INPUT)
  {
    receiverBoardNumber = msg.can_id & FILTER_BOARD_NUMBER;                      // extract the receiver
    senderBoardNumber = (msg.can_id >> BOARD_NUMBER_BITS) & FILTER_BOARD_NUMBER; // extract the sender
    if (receiverBoardNumber==0 && msg.can_dlc==1 && msg.data[0] == RESTART)
    {
      Data::setRestart(true);
      Serial.println("Restarting");
      return true;
    }
  }
  return false;
}


// Checks the commands for a match
void Parser::actuateCommand(String* wordsArray){
  float timer = micros();
  if(source==SERIAL_INPUT)
  {
    local_msg.data[0] = NONE; //message identifier to none
  }

  // Wake up identification
  tryWakeUp(local_msg);

  // "r" Restart
  if (tryRestart(wordsArray, local_msg)){
    return;
  }
  
  // “d <i> <val>” Set duty cycle
  if (trySetCommandFloat(wordsArray,local_msg,SET_DUTY_CYCLE,"d",&Data::setDutyCycle,0,100)){
    return;
  }
  
  // “g d <i>” Get duty cycle
  if (tryGetCommandFloat(wordsArray,local_msg,GET_DUTY_CYCLE,"d",&Data::getDutyCycle)){
    return;
  }
  
  // “r <i> <val>” Set illuminance reference
  if (trySetCommandFloat(wordsArray,local_msg,SET_LUX_REFERENCE,"r",&Data::setReference,0,999)){
    return;
  }
  
  // “g r <i>” Get illuminance reference
  if (tryGetCommandFloat(wordsArray,local_msg,GET_LUX_REFERENCE,"r",&Data::getReference)){
    return;
  }

  // “g l <i>” Get illuminance at luminaire <i>
  if (tryGetCommandFloat(wordsArray,local_msg,GET_LUX_MEASUREMENT,"l",&Data::getIllumminance)){
    return;
  }

  // “o <i> <val>” Set current occupancy state at desk <i>
  if (trySetCommandBool(wordsArray,local_msg,SET_OCCUPANCY,"o",&Data::setOccupancy)){
    return;
  }
  
  // “g o <i>” Get current occupancy state at desk <i>
  if (tryGetCommandBool(wordsArray,local_msg,GET_OCCUPANCY,"o",&Data::getOccupancy)){
    return;
  }

  // “a <i> <val>” Set anti-windup state at desk <i>
  if (trySetCommandBool(wordsArray,local_msg,SET_ANTI_WINDUP,"a",&Data::setWindUp)){
    return;
  }
  
  // “g a <i>” Get anti-windup state at desk <i>
  if (tryGetCommandBool(wordsArray,local_msg,GET_ANTI_WINDUP,"a",&Data::getWindUp)){
    return;
  }
  
  // “k <i> <val>” Set feedback on/off at desk <i>
  if (trySetCommandBool(wordsArray,local_msg,SET_FEEDBACK,"k",&Data::setFeedback)){
    return;
  }
  
  // “g k <i>” Get feedback on/off at desk <i>
  if (tryGetCommandBool(wordsArray,local_msg,GET_FEEDBACK,"k",&Data::getFeedback)){
    return;
  }

  // “g r <i>” Get illuminance reference
  if (tryGetCommandFloat(wordsArray,local_msg,GET_BOARD,"kk",&Data::getBoardNumberInt)){
    return;
  }

  //count time:
  actuationInterval = micros()-timer;
  return;
}
