#ifndef CAN_H_
#define CAN_H_
#include <stdint.h>
#include <Arduino.h>
#include "mcp2515.h"


class CustomCAN {
  public:
    // Constructor
    CustomCAN();
    
    void setupCAN(gpio_irq_callback_t callback);
    void ReadMessage();
    void stateMachineCAN(uint8_t node_address);

    void setDataAvailable(bool data);
    bool getDataAvailable() const;
    bool getTime2Write() const;
   

  private:
    struct can_frame canMsgTx, canMsgRx;
    unsigned long counterTx, counterRx;
    MCP2515::ERROR err;
    unsigned long time_to_write;
    unsigned long write_delay;
    volatile byte data_available;
    const byte interruptPin;

    MCP2515 can0 {spi0, 17, 19, 16, 18, 10000000};
};

#endif /* CAN_H_ */
