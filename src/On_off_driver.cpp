/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "On_off_driver.h"


On_off_driver::On_off_driver(){
    count = 0;
    delay = 10;
    state = 0;
    new_state = 0;    
};

On_off_driver::On_off_driver(int new_delay){
    count = 0;
    delay = new_delay;
    state = 0;
    new_state = 0;    
};

void On_off_driver::tic_tac(){
    if ( count > 0 ) {
        count--;
    } else {
        if (new_state != state ) {
            state = new_state;
        };
    };
};

int On_off_driver::get_state(){
        return state;
};

void On_off_driver::set_state(int to_state){
    if (count > 0) {
        if (to_state != new_state ) {
            new_state = to_state;
            count = delay;
        };
    } else { 
        new_state = to_state;
        count = delay;
    };
};
