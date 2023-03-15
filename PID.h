#ifndef PID_H
#define PID_H
class pid {
float I, D, K, Ti, Td, b, h, y_old, N;
public:
explicit pid( float _h, float _K = 1, float b_ = 1,
float Ti_ = 1, float Td_ = 0, float N_ = 10);
~pid() {};
float compute_control( float r, float y);
void housekeep( float r, float y);
};
inline void pid::housekeep( float r, float y ) {
float e = r-y;
I += K*h/Ti*e;
y_old = y;
}
#endif //PID_H
