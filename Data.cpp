#include "Data.h"

// Constructor
Data::Data() 
    : dutyCycle(0), 
      reference(0), 
      illuminance(0.0), 
      occupancy(false), 
      windUp(false), 
      feedback(false), 
      externalIlluminance(0.0), 
      powerConsumption(0.0), 
      lastRestartTime(0.0), 
      streamValues(false), 
      sendLMBuffer(false)
{ }

// Getter functions
int Data::getDutyCycle() const {
    return dutyCycle;
}

int Data::getReference() const {
    return reference;
}

double Data::getIlluminance() const {
    return illuminance;
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

bool Data::getStreamValues() const {
    return streamValues;
}

bool Data::getSendLMBuffer() const {
    return sendLMBuffer;
}

// Setter functions
void Data::setDutyCycle(int duty) {
    dutyCycle = duty;
}

void Data::setReference(int ref) {
    reference = ref;
}

void Data::setIlluminance(double ill) {
    illuminance = ill;
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

void Data::setStreamValues(bool stream) {
    streamValues = stream;
}

void Data::setSendLMBuffer(bool sendLM) {
  sendLMBuffer = sendLM;
}
