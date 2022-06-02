#ifndef ACTUATOR_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define ACTUATOR_H


class Actuator {
private:
    const int ZTOMAX = 4000; // Duration for motor to reach max distance
    int LOCK = 2000; // Duration for motor to Lock
    int UNLOCK = 2100; // Duration for motor to Unlock
    int AIA;
    int AIB;
    int lockStatus = 2;

public:
    Actuator(int pin1, int pin2, int speedL, int speedU);
    void forward(int duration);
    void backward(int duration);
    void lock();
    void unlock();
    void init(bool lock);
    void actuatorInit(int pin1, int pin2, int speedL, int speedU);
    void toggle();
};


#endif
