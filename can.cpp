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
    can0.setNormalMode();
    gpio_set_irq_enabled_with_callback(interruptPin, GPIO_IRQ_EDGE_FALL, true, callback );
    time_to_write = millis() + write_delay;
}

void CustomCAN::stateMachineCAN(uint8_t node_address){
    if( millis() >= time_to_write ) {
        canMsgTx.can_id = node_address;
        canMsgTx.can_dlc = 8;
        unsigned long div = counterTx*10;
        for( int i = 0; i < 8; i++ )
            canMsgTx.data[7-i]='0'+((div/=10)%10);
            noInterrupts();
            err = can0.sendMessage(&canMsgTx);
            interrupts();
        if(true){
            Serial.print("Sending message ");
            Serial.print( counterTx );
            Serial.print(" from node ");
            Serial.println( node_address, HEX );
        }
        counterTx++;
        time_to_write = millis() + write_delay;
    }
    if( data_available ) {
        noInterrupts();
        can_frame frm {canMsgRx}; //local copy
        interrupts();
        if(true){
            Serial.print("Received message number ");
            Serial.print( counterRx++ );
            Serial.print(" from node ");
            Serial.print( frm.can_id , HEX);
            Serial.print(" : ");
            for (int i=0 ; i < frm.can_dlc ; i++)
                Serial.print((char) frm.data[i]);
            Serial.println(" ");
        }
        data_available = false;
    }
}
// CAN communications
  


void CustomCAN::ReadMessage(){
    can0.readMessage(&canMsgRx);
}

void CustomCAN::setDataAvailable(bool data){
    data_available = data;
}


bool CustomCAN::getDataAvailable() const{
    return data_available;
}