/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   On_off_driver.h
 * Author: user
 *
 * Created on January 20, 2019, 10:43 PM
 */

#ifndef ON_OFF_DRIVER_H
#define ON_OFF_DRIVER_H

class On_off_driver {
public:
    On_off_driver();
    On_off_driver(int new_delay);
    virtual ~On_off_driver(){

    };
    void tic_tac();
    int get_state();
    void set_state(int to_state);
    void stop();
private:
    int count;
    int delay;
    int state;
    int new_state;
};

#endif /* ON_OFF_DRIVER_H */
