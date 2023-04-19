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

    Node(int number_of_boards) {
        c.resize(number_of_boards);
        y.resize(number_of_boards);
        d_av.resize(number_of_boards);
        k.resize(number_of_boards);
        for(int i = 0; i < number_of_boards; i++) {
            k[i].resize(number_of_boards);
        }
        L.resize(number_of_boards);
        o.resize(number_of_boards);
    }
};


#endif