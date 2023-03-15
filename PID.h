#ifndef PID_H
#define PID_H
class pid {
float I, D, K, Ti, Td, b, h, y_old, N,Tt;
public:
explicit pid( float _h, float _K = 10, float b_ = 0,
float Ti_ = 0, float Td_ = 0, float N_ = 10, float Tt_ = 0.01);
~pid() {};
float compute_control( float r, float y);
void housekeep( float r, float y);
};
inline void pid::housekeep( float r, float y ) {
//float e = r- y;
//I += K*h/Ti*e+ao*(compute_control(r,y));
y_old = y;
}
#endif //PID_H