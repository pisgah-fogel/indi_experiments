#ifndef EQMOUNT_HPP
#define EQMOUNT_HPP

#include <CurieTimerOne.h>

#define STEPPER_PIN_STEP 3
#define STEPPER_PIN_DIR 5
#define STEPPER_PIN_DISABLE 6
#define STEPPER_PIN_MICRO 4

CurieTimer timer = CurieTimer();
unsigned long newPeriod = MAX_PERIOD_HZ;

bool timer_running = false;

void callback_timer() {
    // Do one step
    digitalWrite(STEPPER_PIN_STEP, HIGH);
    delayMicroseconds(1000);
    // delayMicroseconds(1) = 3.66us
    // delayMicroseconds(10) = 12.4us
    digitalWrite(STEPPER_PIN_STEP, LOW);
    CurieTimerOne.setPeriod(newPeriod);
}

void eq_setup() {
    // Motor's STEP pin, toogled by timer
    pinMode(STEPPER_PIN_STEP, OUTPUT);
    digitalWrite(STEPPER_PIN_STEP, LOW);

    CurieTimerOne.start(MAX_PERIOD_HZ, &callback_timer);
    Serial.println("Initialized");
    CurieTimerOne.stop();
    timer_running = false;

    pinMode(STEPPER_PIN_DIR, OUTPUT);
    digitalWrite(STEPPER_PIN_DIR, HIGH); // clockwise

    pinMode(STEPPER_PIN_DISABLE, OUTPUT);
    digitalWrite(STEPPER_PIN_DISABLE, HIGH); // Do not power motor

    pinMode(STEPPER_PIN_MICRO, OUTPUT);
    digitalWrite(STEPPER_PIN_MICRO, HIGH); // Full steps
    //digitalWrite(STEPPER_PIN_MICRO, LOW); // x32 micro steps
}

void eq_gotospeed(unsigned long period) {
    digitalWrite(STEPPER_PIN_DISABLE, LOW); // Power motor

    if (!timer_running) {
        timer_running = true;
        // setPeriod(1000): 1.002ms
        // Start from 0 rpm: choose a big enough period
        newPeriod = 100000;
        CurieTimerOne.setPeriod(newPeriod);
        CurieTimerOne.start(); 
    } else {
        newPeriod = period;
    }
}

#endif // EQMOUNT_HPP