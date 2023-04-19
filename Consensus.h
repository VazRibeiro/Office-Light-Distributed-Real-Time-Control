#ifndef CONSENSUS_H_
#define CONSENSUS_H_
#include <Arduino.h>
#include <vector>


struct Node {
    int index;
    std::vector<double> c;
    std::vector<double> y;
    std::vector<double> d_av;
    std::vector<std::vector<double>> k;
    std::vector<double> L;
    std::vector<double> o;
};


#endif