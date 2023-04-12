#include "can.h"

// Constructor
CustomCAN::CustomCAN() 
  : counterTx {0},
    counterRx {0}, 
    write_delay {1000}, 
    data_available {false},
    data_received {false},
    interruptPin {20}
{ }

void CustomCAN::setupCAN(gpio_irq_callback_t callback){
    can0.reset();
    can0.setBitrate(CAN_1000KBPS);
    can0.setNormalMode();
    gpio_set_irq_enabled_with_callback(interruptPin, GPIO_IRQ_EDGE_FALL, true, callback);
    time_to_write = millis() + write_delay;
}

void CustomCAN::ReadMessage(){
    can0.readMessage(&canMsgRx);
}

void CustomCAN::setDataAvailable(bool dataAvailable){
    data_available = dataAvailable;
}

void CustomCAN::setDataReceived(bool dataReceived){
    data_received = dataReceived;
}

bool CustomCAN::getDataReceived() const {
    return data_received;
}

can_frame CustomCAN::getcanMsgRx() const {
    return canMsgRx;
}