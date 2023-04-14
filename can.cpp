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
    can0.setFilterMask(MCP2515::MASK0, 0, 0x0000000F);
    can0.setFilter(MCP2515::RXF0, 0, 0);
    can0.setFilter(MCP2515::RXF1, 0, boardNumber);
    can0.setNormalMode();
    gpio_set_irq_enabled_with_callback(interruptPin, GPIO_IRQ_EDGE_FALL, true, callback);
    time_to_write = millis() + write_delay;
}

void CustomCAN::ReadMessage(){
    can0.readMessage(&canMsgRx);
}

void CustomCAN::SendMessage(const can_frame *msg){
    can0.sendMessage(msg);
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