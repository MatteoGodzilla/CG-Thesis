#include "flag.h"

Flag::Flag(bool initial){
    state = initial; 
}

void Flag::set(){
    state = true;
}

void Flag::clear(){
    state = false;
}

bool Flag::getState(){
    return state;
}
