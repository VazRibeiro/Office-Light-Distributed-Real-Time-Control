#ifndef DATA_H
#define DATA_H
#include <Arduino.h>

class Data {
private:
  String boardNumber;
  float dutyCycle;
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

public:
  // Constructor
  Data();

  // Getter functions
  String getBoardNumber() const;
  float getDutyCycle() const;
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

  // Setter functions
  void setBoardNumber(String boardNumb);
  void setDutyCycle(float duty);
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
};

#endif
