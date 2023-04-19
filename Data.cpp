#include "Data.h"

// Constructor
Data::Data() 
  : boardNumber(0),
    dutyCycle(),
    illuminance(),
    node(),
    reference(0), 
    occupancy(false), 
    windUp(false), 
    feedback(false), 
    externalIlluminance(0.0), 
    powerConsumption(0.0), 
    lastRestartTime(0.0), 
    illuminanceStreamValues(false),
    dutyCycleStreamValues(false),
    sendLMBuffer(false),
    restart(false),
    calibrationOver(false)
{ }

// Getter functions //////////////////////////////////////////
String Data::getBoardNumber() const {
  return boardNumber;
}

float Data::getBoardNumberInt() const {
  return boardNumber.toFloat();
}

int Data::getTotalNumberOfBoards() const {
  return node.size();
}

float Data::getDutyCycle() const {
  int recent_index = (dutyCycle.getHead() + dutyCycle.capacity() - 1) % dutyCycle.capacity();
  return dutyCycle.read(recent_index);
}

float Data::getIllumminance() const {
  int recent_index = (illuminance.getHead() + illuminance.capacity() - 1) % illuminance.capacity();  
  return illuminance.read(recent_index);
}

int Data::getNode(int index) const {  
  return node.read(index);
}

float Data::getReference() const {
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

bool Data::getRestart() const {
  return restart;
}

bool Data::getCalibrationOver() const {
  return calibrationOver;
}


// Setter functions ///////////////////////////////////////////////

void Data::setBoardNumber(String boardNumb) {
  boardNumber = boardNumb;
}

void Data::setDutyCycle(float duty) {
  dutyCycle.put(duty);
}

void Data::setIlluminance(float illum) {
  illuminance.put(illum);
}

void Data::setNode(int nd) {
  node.put(nd);
}

void Data::setReference(float ref) {
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

void Data::setRestart(bool rstrt) {
  restart = rstrt;
}

void Data::setCalibrationOver(bool calib) {
  calibrationOver = calib;
}