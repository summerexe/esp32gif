#include "ServoController.h"
#include <ESP32PWM.h>

void ServoController::begin(int pitchPin, int yawPin, int rollPin) {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    servoPitch.setPeriodHertz(50);
    servoYaw.setPeriodHertz(50);
    servoRoll.setPeriodHertz(50);

    servoPitch.attach(pitchPin, 500, 2400);
    servoYaw.attach(yawPin,   500, 2400);
    servoRoll.attach(rollPin, 500, 2400);

    pitchNeutral = 1500;
    pitchDown    = 1750;
    pitchUp      = 1250;

    yawLeft  = 1100;
    yawRight = 1900;

    rollLeft  = 1400;
    rollRight = 1600;

    // Initial positions
    setPitch(pitchNeutral);
    setYaw((yawLeft + yawRight) / 2);
    setRoll((rollLeft + rollRight) / 2);
}

void ServoController::setPitch(int usec) { servoPitch.writeMicroseconds(usec); }
void ServoController::setYaw(int usec)   { servoYaw.writeMicroseconds(usec);   }
void ServoController::setRoll(int usec)  { servoRoll.writeMicroseconds(usec);  }
