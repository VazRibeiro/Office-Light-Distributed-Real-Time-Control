#include "can.h"

// Constructor
CustomCAN::CustomCAN() 
  : counterTx {0},
    counterRx {0}, 
    write_delay {1000}, 
    data_available {false},
    interruptPin {20}
{ }

void CustomCAN::setupCAN(gpio_irq_callback_t callback, int boardNumber){
    can0.reset();
    can0.setBitrate(CAN_1000KBPS);
    can0.setFilterMask(MCP2515::MASK0, true, 0x0000000F);
    can0.setFilter(MCP2515::RXF0, true, 0);
    can0.setFilter(MCP2515::RXF1, true, boardNumber);
    can0.setFilterMask(MCP2515::MASK1, true, 0x0000000F);
    can0.setFilter(MCP2515::RXF2, true, 0);
    can0.setFilter(MCP2515::RXF3, true, boardNumber);
    can0.setNormalMode();
    gpio_set_irq_enabled_with_callback(interruptPin, GPIO_IRQ_EDGE_FALL, true, callback);
    time_to_write = millis() + write_delay;
}

void CustomCAN::ReadMessage(){
    can0.readMessage(&canMsgRx);
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

void CustomCAN::setDataAvailable(bool dataAvailable){
    data_available = dataAvailable;
}

bool CustomCAN::getDataAvailable() const {
    return data_available;
}

can_frame CustomCAN::getcanMsgRx() const {
    return canMsgRx;
}

void CustomCAN::SendWakeUpMessage(int node_adress, int FILTER_BOARD_NUMBER, int BOARD_NUMBER_BITS){
    can_frame canMsgTx;
    canMsgTx.can_id =   (0 & FILTER_BOARD_NUMBER) |
                        ((node_adress & FILTER_BOARD_NUMBER) << BOARD_NUMBER_BITS) |
                        CAN_EFF_FLAG;
    canMsgTx.can_dlc = 0;
    SendMessage(&canMsgTx);
}