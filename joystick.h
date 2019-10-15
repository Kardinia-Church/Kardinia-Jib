/**
    Main Joystick Handler
    Responsible for handling the joystick

    There is something hardware wise wrong with the z axis
**/

#ifndef JOYSTICK_HANDLER
#define JOYSTICK_HANDLER

#include <Arduino.h>

#define X_PIN A5
#define Y_PIN A6
#define Z_PIN A7
#define SPEED_POT_PIN A1

#define X_CENTER 521
#define X_MAXVALUE 160
#define Y_CENTER 519
#define Y_MAXVALUE 155
#define Z_CENTER 514
#define Z_MAXVALUE 5
#define DEADZONE 10
#define SPEED_POT_MAX 1023.0

void joystickSetup() {
    Serial.println("[SETUP] - Joystick");
    pinMode(X_PIN, INPUT);
    pinMode(Y_PIN, INPUT);
    pinMode(Z_PIN, INPUT);
    pinMode(SPEED_POT_PIN, INPUT);
}

//Calculate the joystick position as a raw value
int joystickCalculateValue(int pin, int center, int deadzone) {
    //Get multiple readings to make it more precise and reduce bounce
    int value = analogRead(pin);

    int val = center - value;
    if(val > deadzone) {
        val = val - deadzone;
    }
    else if(val < -deadzone) {
        val = val + deadzone;
    }
    else {val = 0;}
    return val;
}

//Calculate the percentage of a value
float calculatePercentage(float value, float maxValue) {
    float val = (value/maxValue) * 100.0;
    if(val < -100.0){val = -100.0;}
    if(val > 100.0) {val = 100.0;}
    return val;
}

//Output the joystick values
void joystickDebug() {
    int value = 0;
    float percentage = 0.0;
    Serial.print("[DEBUG][JOY] - XYZ:");
    Serial.print(analogRead(X_PIN));
    Serial.print(", ");
    Serial.print(analogRead(Y_PIN));
    Serial.print(", ");
    Serial.print(analogRead(Z_PIN));
    Serial.print("; VAL XYZ:");
    value = joystickCalculateValue(X_PIN, X_CENTER, DEADZONE);
    percentage = calculatePercentage(value, X_MAXVALUE);
    Serial.print((String)percentage + "(" + value + "), ");
    
    value = joystickCalculateValue(Y_PIN, Y_CENTER, DEADZONE);
    percentage = calculatePercentage(value, Y_MAXVALUE);
    Serial.print((String)percentage + "(" + value + "), ");

    value = joystickCalculateValue(Z_PIN, Z_CENTER, DEADZONE);
    percentage = calculatePercentage(value, Z_MAXVALUE);
    Serial.print((String)percentage + "(" + value + ")");

    Serial.println(";");
}

float speedPotDivisor() {
    return map((float)analogRead(SPEED_POT_PIN) / SPEED_POT_MAX, 0, 0.4, 0.0, 1.0);
}

//Output the current X value as a percentage
float joystickXPercentage() {
    return calculatePercentage(joystickCalculateValue(X_PIN, X_CENTER, DEADZONE), X_MAXVALUE) * speedPotDivisor();
}
//Output the current Y value as a percentage
float joystickYPercentage() {
    return calculatePercentage(joystickCalculateValue(Y_PIN, Y_CENTER, DEADZONE), Y_MAXVALUE) * speedPotDivisor();
}

//Output the current Z value as a percentage
float joystickZPercentage() {
    return calculatePercentage(joystickCalculateValue(Z_PIN, Z_CENTER, DEADZONE), Z_MAXVALUE) * speedPotDivisor();
}

#endif