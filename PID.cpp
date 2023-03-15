#include "pid.h"
pid::pid( float _h, float _K, float b_,
float Ti_, float Td_, float N_, float Tt_)
// member variable initialization list
: h {_h}, K {_K}, b {b_}, Ti {Ti_}, Td {Td_},
N {N_}, I {0.0}, D {0.0}, y_old{0.0},  Tt {Tt_}
{ } // should check arguments validity
float pid::compute_control( float r, float y ) {
float P = K*(b*r-y);
float ad = Td/(Td+N*h);
float bd = Td*K*N/(Td+N*h);
float ao = h/Tt;
D = ad*D-bd*(y-y_old);
float v = P+I+D;
float u;
if( v < 0 ) u = 0;
if( v > 4095 ) u = 4095;

float e = r-y;
I += K*h/Ti*e+ao*(u-v);
return u;
}