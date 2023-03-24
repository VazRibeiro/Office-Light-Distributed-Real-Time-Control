#ifndef DATA_H
#define DATA_H

class Data {
private:
  int boardNumber;
  int nodeID;
  int dutyCycle;
  int reference;
  double illuminance;
  bool occupancy;
  bool windUp;
  bool feedback;
  double externalIlluminance;
  double powerConsumption;
  double lastRestartTime;
  bool streamValues;
  bool sendLMBuffer;

public:
  // Constructor
  Data();

  // Getter functions
  int getDutyCycle() const;
  int getReference() const;
  double getIlluminance() const;
  bool getOccupancy() const;
  bool getWindUp() const;
  bool getFeedback() const;
  double getExternalIlluminance() const;
  double getPowerConsumption() const;
  double getLastRestartTime() const;
  bool getStreamValues() const;
  bool getSendLMBuffer() const;

  // Setter functions
  void setDutyCycle(int duty);
  void setReference(int ref);
  void setIlluminance(double ill);
  void setOccupancy(bool occ);
  void setWindUp(bool wu);
  void setFeedback(bool fb);
  void setExternalIlluminance(double extIll);
  void setPowerConsumption(double powCons);
  void setLastRestartTime(double time);
  void setStreamValues(bool stream);
  void setSendLMBuffer(bool sendLM);
};

#endif
