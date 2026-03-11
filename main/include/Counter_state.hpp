#ifndef COUNTER_STATE_HPP
#define COUNTER_STATE_HPP

struct counter_modelState {
    double sigma;
    int count;
    int increment;
    bool countUp;

    explicit counter_modelState(): sigma(1), count(0), increment(1), countUp(true){
    }
};

#endif