#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <random>
#include <iostream>
#include "cadmium/modeling/devs/atomic.hpp"
#include "Counter_state.hpp"
#include "Counter_functions.hpp"

using namespace cadmium;

std::ostream& operator<<(std::ostream &out, const counter_modelState& state) {
    out  << "sigma:" << state.sigma << ";count:" << state.count << ";increment:" << state.increment << ";countUp:" << state.countUp;
    return out;
}

class Counter : public Atomic<counter_modelState> {
    public:

    Port<bool> direction_in;
    Port<int> increment_in;
    Port<int> count_out;

    Counter(const std::string id) : Atomic<counter_modelState>(id, counter_modelState()) {
        direction_in = addInPort<bool>("direction_in");
        increment_in = addInPort<int>("increment_in");
        count_out = addOutPort<int>("count_out");
    }

    // inernal transition
    void internalTransition(counter_modelState& state) const override {
        internal_transition_body(&state);
    }

    // external transition
    void externalTransition(counter_modelState& state, double e) const override {
        external_transition_body(&state, direction_in, increment_in);
    }

    void confluentTransition(counter_modelState& state, double e) const override {
        externalTransition(state, e);
        internalTransition(state);
    }
    
    
    // output function
    void output(const counter_modelState& state) const override {
        output_body(&state, count_out);
    }

    // time_advance function
    [[nodiscard]] double timeAdvance(const counter_modelState& state) const override {     
        return time_advance_body(&state);
    }
};

#endif