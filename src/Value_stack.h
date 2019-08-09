#ifndef _VALUE_STACK_H_
#define _VALUE_STACK_H_

#define SIZE_OF_STACK 7

class Value_stack {
public:
    int get_average();
    void add_value(int new_value);
    Value_stack();
    virtual ~Value_stack();
private:
    int a[SIZE_OF_STACK];
    int index;
};

#endif
