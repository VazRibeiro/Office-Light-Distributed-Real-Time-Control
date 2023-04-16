#include "pid.h"
pid::pid( float _h,
    float _K,
    float b_,
    float Ti_,
    float Td_,
    float N_,
    float Tt_,
    float bold_,
    float kold_
)
// member variable initialization list
    : h {_h},
    K {_K}, 
    b {b_}, 
    Ti {Ti_}, 
    Td {Td_},
    N {N_}, 
    Tt{Tt_},
    I {0.0}, 
    D {0.0}, 
    y_old{0.0},
    bold{0.0},
    kold{0.0}
{ } // should check arguments validity

float pid::compute_control( float r, float y ) {
  float P = K*(b*r-y);
  float ad = Td/(Td+N*h);
  float bd = Td*K*N/(Td+N*h);
  D = ad*D-bd*(y-y_old);
  I = I + kold*(bold*r-y)-K*(b*r-y);
  float u = P+I+D;
  return u;
}

float pid::saturate_output( float v ) {
  float u = v;
  if( u < 0 ) u = 0;
  else if( u > 4095 ) u = 4095;
  return u;
}
