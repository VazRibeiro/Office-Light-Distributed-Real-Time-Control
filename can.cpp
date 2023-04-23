#include "can.h"

// Constructor
CustomCAN::CustomCAN() 
  : counterTx {0},
    counterRx {0}, 
    write_delay {1000}, 
    data_available {false},
    interruptPin {20}
{ }

void CustomCAN::setupCAN(gpio_irq_callback_t callback){
    can0.reset();
    can0.setBitrate(CAN_1000KBPS);
    can0.setFilterMask(MCP2515::MASK0, true, 0x3FFF);
    can0.setFilter(MCP2515::RXF0, true, 0);
    //can0.setFilter(MCP2515::RXF1, true, boardNumber);
    // can0.setFilterMask(MCP2515::MASK1, true, 0x3FFF);
    // can0.setFilter(MCP2515::RXF2, true, 0);
    //can0.setFilter(MCP2515::RXF3, true, boardNumber);
    can0.setNormalMode();
    gpio_set_irq_enabled_with_callback(interruptPin, GPIO_IRQ_EDGE_FALL, true, callback);
    time_to_write = millis() + write_delay;
}

void CustomCAN::setupFilters(int boardNumber){
    Serial.println("Updated filters...");
    can0.setFilter(MCP2515::RXF0, true, 0);
    can0.setFilter(MCP2515::RXF1, true, boardNumber);
    // can0.setFilter(MCP2515::RXF2, true, 0);
    // can0.setFilter(MCP2515::RXF3, true, boardNumber);
    can0.setNormalMode();
}

MCP2515::ERROR CustomCAN::ReadMessage(){
    struct can_frame msg;
    MCP2515::ERROR err;
    err = can0.readMessage(&msg);
    canMsgRx.put(msg);
    can0.clearInterrupts();
    return err;
}

void CustomCAN::SendMessage(const can_frame *msg){
    if (can0.sendMessage(msg) != MCP2515::ERROR_OK)
    {
        Serial.println("Error sending message");
    }
    else
    {
        Serial.println("Message sent successfully");
    }
}

void CustomCAN::setDataAvailable(int dataAvailable){
    data_available = dataAvailable;
}

int CustomCAN::getDataAvailable() const {
    return data_available;
}

void CustomCAN::decrementDataAvailable(){
    data_available--;
}

void CustomCAN::incrementDataAvailable(){
    data_available++;
}

can_frame CustomCAN::getcanMsgRx() {
    return canMsgRx.take();
}

bool CustomCAN::isEmptycanMsgRx() {
    return canMsgRx.is_empty();
}

void CustomCAN::SendWakeUpMessage(
    int board_number,
    int FILTER_BOARD_NUMBER,
    int BOARD_NUMBER_BITS,
    int msg_identifier)
{
    can_frame canMsgTx;
    canMsgTx.can_id =   (0 & FILTER_BOARD_NUMBER) | //to everyone
                        ((board_number & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                        CAN_EFF_FLAG;
    canMsgTx.can_dlc = 1;
    canMsgTx.data[0] = msg_identifier;
    noInterrupts();
    SendMessage(&canMsgTx);
    interrupts();
}

void CustomCAN::SendCalibrationMessage(
    int board_number,
    int FILTER_BOARD_NUMBER,
    int BOARD_NUMBER_BITS,
    int RESPONSE_FLAG,
    int FILTER_MESSAGE_TYPE,
    int msg_identifier)
{
    can_frame canMsgTx;
    canMsgTx.can_id =   (0 & FILTER_BOARD_NUMBER) | //to everyone
                        ((1 & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) | //from board 1 especifically
                        ((RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) | //0, is a command
                        CAN_EFF_FLAG;
    canMsgTx.can_dlc = sizeof(int)+1;
    canMsgTx.data[0] = msg_identifier;
    int val = board_number;
    memcpy(&canMsgTx.data[1],&val,sizeof(int));
    noInterrupts();
    SendMessage(&canMsgTx);
    interrupts();
}

void CustomCAN::SendCalibrationValues(
    int sender,
    int calibrationFlag,
    int FILTER_BOARD_NUMBER,
    int BOARD_NUMBER_BITS,
    int RESPONSE_FLAG,
    int FILTER_MESSAGE_TYPE,
    int msg_identifier,
    float k)
{
    can_frame canMsgTx;
    canMsgTx.can_id =   (0 & FILTER_BOARD_NUMBER) | //to everyone
                        ((sender & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) | //from a specific board
                        ((RESPONSE_FLAG & FILTER_MESSAGE_TYPE) << (BOARD_NUMBER_BITS*2)) | //1, is a response
                        CAN_EFF_FLAG;
    canMsgTx.can_dlc = sizeof(float)+1;
    canMsgTx.data[0] = msg_identifier;
    float val = k;
    memcpy(&canMsgTx.data[1],&val,sizeof(float));
    canMsgTx.data[5] = calibrationFlag;
    noInterrupts();
    SendMessage(&canMsgTx);
    interrupts();
}

void CustomCAN::checkERRORs(){
    uint8_t eflags = can0.getErrorFlags();
    if(eflags & MCP2515::EFLG_RX0OVR) Serial.println("RX Buffer 0 Overflow");
    if(eflags & MCP2515::EFLG_RX1OVR ) Serial.println("RX Buffer 1 Overflow");
    //Clear Read Buffer Overflow Error Condition
    can0.clearRXnOVRFlags();
}