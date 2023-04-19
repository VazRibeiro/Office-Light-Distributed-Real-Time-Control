#ifndef PID_H
#define PID_H
#include <Arduino.h>

class pid
{
float I, D, K, Ti, Td, b, h, y_old, N, Tt, bold, kold;

public:
explicit pid( float _h, float _K = 1, float b_ = 1,
float Ti_ = 1, float Td_ = 0, float N_ = 10, float Tt_ = 0.01 , float bold_ = 0, float kold_ = 0);
~pid() {};
float compute_control( float r, float y);
void housekeep( float r, float y, float v, float u, bool w);
float saturate_output( float v );
};


inline void pid::housekeep( float r, float y , float v, float u, bool w) {
  float e = r-y;
  if(!w){
  I = I + K*h/Ti*e;}
  if(w){
  I = I + K*h/Ti*e + (h/Tt)*(u-v);}
  y_old = y;
  kold = K;
  bold = b;
}
#endif //PID_H