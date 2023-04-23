#ifndef CAN_H_
#define CAN_H_
#include <stdint.h>
#include <Arduino.h>
#include "mcp2515.h"
#include "Buffer.h"

class CustomCAN {
  public:
    // Constructor
    CustomCAN();
    
    void setupCAN(gpio_irq_callback_t callback);
    void setupFilters(int boardNumber);
    MCP2515::ERROR ReadMessage();
    void SendMessage(const can_frame *msg);
    void SendWakeUpMessage(int board_number, int FILTER_BOARD_NUMBER, int BOARD_NUMBER_BITS, int msg_identifier);
    void SendCalibrationMessage(
      int board_number, 
      int FILTER_BOARD_NUMBER, 
      int BOARD_NUMBER_BITS,
      int RESPONSE_FLAG,
      int FILTER_MESSAGE_TYPE, 
      int msg_identifier);
    void SendCalibrationValues(
      int sender,
      int calibrationFlag,
      int FILTER_BOARD_NUMBER,
      int BOARD_NUMBER_BITS,
      int RESPONSE_FLAG,
      int FILTER_MESSAGE_TYPE,
      int msg_identifier,
      float k);
    void setDataAvailable(int data);
    int getDataAvailable() const;
    void decrementDataAvailable();
    void incrementDataAvailable();
    can_frame getcanMsgRx();
    bool isEmptycanMsgRx();
    void checkERRORs();


  private:
    circular_buffer<can_frame,100> canMsgRx;
    unsigned long counterTx, counterRx;
    MCP2515::ERROR err;
    unsigned long time_to_write;
    unsigned long write_delay;
    volatile byte data_available;
    const byte interruptPin;
    
    MCP2515 can0 {spi0, 17, 19, 16, 18, 10000000};
};

#endif /* CAN_H_ */