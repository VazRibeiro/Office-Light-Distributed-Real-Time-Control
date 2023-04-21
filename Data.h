#ifndef DATA_H
#define DATA_H
#include <Arduino.h>
#include "Buffer.h"

class Data {
private:
  String boardNumber;
  circular_buffer<float, 6000> dutyCycle;
  circular_buffer<float, 6000> illuminance;
  circular_buffer<int,16384> node;
  float reference;
  bool occupancy;
  bool windUp;
  bool feedback;
  double externalIlluminance;
  double powerConsumption;
  double lastRestartTime;
  bool illuminanceStreamValues;
  bool dutyCycleStreamValues;
  bool sendLMBuffer;
  bool restart;
  int  calibrationFlag;
  bool calibrationAcknowledge;
  int timeout;

public:
  // Constructor
  Data();
  void reset() {
    boardNumber = "";
    dutyCycle.clear();
    dutyCycle.put(0);
    illuminance.clear();
    illuminance.put(0);
    node.clear();
    reference = 0;
    occupancy = false;
    windUp = true;
    feedback = true;
    externalIlluminance = 0.0;
    powerConsumption = 0.0;
    lastRestartTime = 0.0;
    illuminanceStreamValues = false;
    dutyCycleStreamValues = false;
    sendLMBuffer = false;
    restart = false;
    calibrationFlag = 0;
    calibrationAcknowledge = false;
    timeout = 0;
  }
  void clearNodeBuffer() { node.clear();}
  void incrementcalibrationFlag() { calibrationFlag++;}

  // Getter functions
  String getBoardNumber() const;
  float getBoardNumberInt() const;
  int getTotalNumberOfBoards() const;
  float getDutyCycle() const; 
  float getIllumminance() const;
  int getNode(int index) const;
  float getReference() const;
  bool getOccupancy() const;
  bool getWindUp() const;
  bool getFeedback() const;
  double getExternalIlluminance() const;
  double getPowerConsumption() const;
  double getLastRestartTime() const;
  bool getIlluminanceStreamValues() const;
  bool getDutyCycleStreamValues() const;
  bool getSendLMBuffer() const;
  bool getRestart() const;
  int getcalibrationFlag() const;
  bool getcalibrationAcknowledge() const;
  int getTimeout() const;

  // Setter functions
  void setBoardNumber(String boardNumb);
  void setDutyCycle(float duty);
  void setIlluminance(float illum);
  void setNode(int nd);
  void setReference(float ref);
  void setOccupancy(bool occ);
  void setWindUp(bool wu);
  void setFeedback(bool fb);
  void setExternalIlluminance(double extIll);
  void setPowerConsumption(double powCons);
  void setLastRestartTime(double time);
  void setIlluminanceStreamValues(bool streamI);
  void setDutyCycleStreamValues(bool streamD);
  void setSendLMBuffer(bool sendLM);
  void setRestart(bool rstrt);
  void setcalibrationFlag(int calib);
  void setcalibrationAcknowledge(bool ackn);
  void setTimeout(int timeo);
};

#endif
