#ifndef COUNTER_FUNCTIONS_HPP
#define COUNTER_FUNCTIONS_HPP

#include "Counter_state.hpp"

using namespace cadmium;

void internal_transition_body(counter_modelState* state){
    if (state->countUp){
        state->count += state->increment;
    } else {
        state->count -= state->increment;
    }
}

void external_transition_body(counter_modelState* state, Port<bool> direction_in, Port<int> increment_in){
    if (!direction_in->empty()){
        for (const auto x: direction_in->getBag()){
            state->countUp = x;
        }
    }
    if (!increment_in->empty()){
        for (const auto x: increment_in->getBag()){
            state->increment = x;
        }
    }
}

void output_body(const counter_modelState* state, Port<int> count_out){
    count_out->addMessage(state->count);
}

double time_advance_body(const counter_modelState* state){
    return state->sigma;
}

#endif