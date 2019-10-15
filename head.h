/**
    Main head handler
    Responsible for handling movement in the head
**/

#ifndef HEAD_HANDLER
#define HEAD_HANDLER

#include <Arduino.h>
#include <AccelStepper.h>

#define DEFAULT_ACCELERATION 10000
#define ACCEL_POT_PIN A0
#define ACCEL_POT_MAX 1023

#define X_DIR_PIN 2
#define X_STEP_PIN 3
#define X_LIMIT_PIN 50
#define X_INVERT 1
#define X_MAX_SPEED 1400
#define X_MAX_POSITION 6000

#define Y_DIR_PIN 4
#define Y_STEP_PIN 5
#define Y_LIMIT_PIN 52
#define Y_INVERT 1
#define Y_MAX_SPEED 600
#define Y_MAX_POSITION 1000

AccelStepper xStepper(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper yStepper(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);

void headSetup() {
    Serial.println("[SETUP] - Head");
    pinMode(ACCEL_POT_PIN, INPUT);
    xStepper.setMaxSpeed(X_MAX_SPEED);
    xStepper.setAcceleration(DEFAULT_ACCELERATION);
    yStepper.setMaxSpeed(Y_MAX_SPEED);
    yStepper.setAcceleration(DEFAULT_ACCELERATION);
    //xStepper.moveTo(500);
    while(xStepper.distanceToGo() != 0) {
        xStepper.run();
        yStepper.run();
    }
}


float getAcceleration() {
    float accel = ((float)analogRead(ACCEL_POT_PIN) / ACCEL_POT_MAX) * 100;
    return map(DEFAULT_ACCELERATION * (accel/10.0), 0, DEFAULT_ACCELERATION, 1, DEFAULT_ACCELERATION / 5);
}

void moveX(float speed) {
    //Set the speed
    if(speed != 0) {
        if(!(xStepper.distanceToGo() != 0 && abs(speed) < abs((xStepper.speed() / X_MAX_SPEED) * 100))) {
            xStepper.setMaxSpeed(abs(X_MAX_SPEED * speed/100.0));
        }
        
        xStepper.setAcceleration(getAcceleration());
    }

    //Set the direction 
    if(speed < 0) {
        if(X_INVERT == 0) {xStepper.move(-X_MAX_POSITION);}
        else {xStepper.move(X_MAX_POSITION);}
    }
    else if(speed > 0){
        if(X_INVERT == 0) {xStepper.move(X_MAX_POSITION);}
        else {xStepper.move(-X_MAX_POSITION);}
    }
    else {
        if(xStepper.distanceToGo() != 0) {
            xStepper.setMaxSpeed(xStepper.speed());
            xStepper.stop();
        }
    }
}

void moveY(float speed) {
    //Set the speed
    if(speed != 0) {
        if(!(yStepper.distanceToGo() != 0 && abs(speed) < abs((yStepper.speed() / Y_MAX_SPEED) * 100))) {
            yStepper.setMaxSpeed(abs(Y_MAX_SPEED * speed/100.0));
        }
        
        yStepper.setAcceleration(getAcceleration());
    }

    //Set the direction 
    if(speed < 0) {
        if(Y_INVERT == 0) {xStepper.move(-Y_MAX_POSITION);}
        else {yStepper.move(Y_MAX_POSITION);}
    }
    else if(speed > 0){
        if(Y_INVERT == 0) {xStepper.move(Y_MAX_POSITION);}
        else {yStepper.move(-Y_MAX_POSITION);}
    }
    else {
        if(yStepper.distanceToGo() != 0) {
            yStepper.setMaxSpeed(yStepper.speed());
            yStepper.stop();
        }
    }
}

void headLoop() {
    xStepper.run();
    yStepper.run();
}

#endif