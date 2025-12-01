#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <ESP32Servo.h>

class ServoController {
public:
    void begin(int pitchPin, int yawPin, int rollPin);

    // Movement functions
    void setPitch(int usec);
    void setYaw(int usec);
    void setRoll(int usec);

    int pitchNeutral, pitchDown, pitchUp;
    int yawLeft, yawRight;
    int rollLeft, rollRight;

private:
    Servo servoPitch;
    Servo servoYaw;
    Servo servoRoll;
};

#endif
