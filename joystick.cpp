// joystick.cpp
#include "joystick.h"

Joystick::Joystick(PinName xPin, PinName yPin, PinName buttonPin)
    : VRx(xPin), VRy(yPin), button(buttonPin, PullUp) {}

float Joystick::readX() {
    return VRx.read();
}

float Joystick::readY() {
    return VRy.read();
}

bool Joystick::isButtonPressed() {
    return !button;
}