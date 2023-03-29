#include "Data.h"

// Constructor
Data::Data() 
  : boardNumber(0),
    dutyCycle(0), 
    reference(0), 
    occupancy(false), 
    windUp(false), 
    feedback(false), 
    externalIlluminance(0.0), 
    powerConsumption(0.0), 
    lastRestartTime(0.0), 
    illuminanceStreamValues(false),
    dutyCycleStreamValues(false),
    sendLMBuffer(false)
{ }

// Getter functions //////////////////////////////////////////
String Data::getBoardNumber() const {
  return boardNumber;
}

int Data::getDutyCycle() const {
  return dutyCycle;
}

int Data::getReference() const {
  return reference;
}

bool Data::getOccupancy() const {
  return occupancy;
}

bool Data::getWindUp() const {
  return windUp;
}

bool Data::getFeedback() const {
  return feedback;
}

double Data::getExternalIlluminance() const {
  return externalIlluminance;
}

double Data::getPowerConsumption() const {
  return powerConsumption;
}

double Data::getLastRestartTime() const {
  return lastRestartTime;
}

bool Data::getIlluminanceStreamValues() const {
  return illuminanceStreamValues;
}

bool Data::getDutyCycleStreamValues() const {
  return dutyCycleStreamValues;
}

bool Data::getSendLMBuffer() const {
  return sendLMBuffer;
}


// Setter functions ///////////////////////////////////////////////

void Data::setBoardNumber(String boardNumb) {
  boardNumber = boardNumb;
}

void Data::setDutyCycle(int duty) {
  dutyCycle = duty;
}

void Data::setReference(int ref) {
  reference = ref;
}

void Data::setOccupancy(bool occ) {
  occupancy = occ;
}

void Data::setWindUp(bool wu) {
  windUp = wu;
}

void Data::setFeedback(bool fb) {
  feedback = fb;
}

void Data::setExternalIlluminance(double extIll) {
  externalIlluminance = extIll;
}

void Data::setPowerConsumption(double powCons) {
  powerConsumption = powCons;
}

void Data::setLastRestartTime(double time) {
  lastRestartTime = time;
}

void Data::setIlluminanceStreamValues(bool streamI) {
  illuminanceStreamValues = streamI;
}

void Data::setDutyCycleStreamValues(bool streamD) {
  dutyCycleStreamValues = streamD;
}

void Data::setSendLMBuffer(bool sendLM) {
  sendLMBuffer = sendLM;
}
