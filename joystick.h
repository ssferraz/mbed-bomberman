// joystick.h
#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "mbed.h"

class Joystick {
private: 
    AnalogIn VRx, VRy;
    DigitalIn button;

public:
    Joystick(PinName xPin, PinName yPin, PinName buttonPin);
    float readX();
    float readY();
    bool isButtonPressed();
};

#endif