// joystick.cpp
#include "joystick.h"

Joystick::Joystick(PinName xPin, PinName yPin, PinName buttonPin)
    : x(xPin), y(yPin), button(buttonPin, PullUp) {}

float Joystick::readX() {
    return x.read();
}

float Joystick::readY() {
    return y.read();
}

bool Joystick::isButtonPressed() {
    return !button;
}
