#include "Value_stack.h"

int Value_stack::get_average(){
    int summ = 0;
    for (int i=0; i<SIZE_OF_STACK; i++) {
      summ = summ + a[i];
    };
    return summ/SIZE_OF_STACK;
};

void Value_stack::add_value(int new_value) {
    if (index == SIZE_OF_STACK) index = 0;
    a[index] = new_value;
    index++;
};

Value_stack::Value_stack(){
    for (int i=0; i<SIZE_OF_STACK; i++) {
      a[i] = 0;
    };
    index = 0;
};

Value_stack::~Value_stack(){

};
