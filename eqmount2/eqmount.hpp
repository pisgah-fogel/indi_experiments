// Uses Arduino Nano's 16bits timer to drive a stepper motor
// though a DRV8825 driver
// Tested with Atmega168 uProcessor
// Tested with Arduino Geniuno 101

#ifndef EQMOUNT_HPP
#define EQMOUNT_HPP

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
 // TODO: add all cards supported by TimerOne
 #include <TimerOne.h>
 #define TIMERONE
 #define MAX_PERIOD_HZ 1000000
#else
 #include <CurieTimerOne.h>
 #warning “If your card is not an Arduino geniuno 101 it is not supported”
 #define CURIE
#endif

#ifndef STEPPER_PIN_STEP
    #error "You should define STEPPER_PIN_STEP in your sketch before including eqmount.hpp"
#endif
#ifndef STEPPER_PIN_DIR
    #error "You should define STEPPER_PIN_DIR in your sketch before including eqmount.hpp"
#endif
#ifndef STEPPER_PIN_DISABLE
    #error "You should define STEPPER_PIN_DISABLE in your sketch before including eqmount.hpp"
#endif
#ifndef STEPPER_PIN_MICRO
    #error "You should define STEPPER_PIN_MICRO in your sketch before including eqmount.hpp"
#endif

#define STEPPER_PERIOD_MIN 100000 // experimental value
#define STEPPER_PROP_ACCELL 0.00000000001 // Acceleration used in _update_newPeriod; experimental value

#define STEPPER_CONST_ACCELL 50 // Acceleration used in _update_newPeriod; microsecond per microsecond

#ifdef CURIE
    CurieTimer timer = CurieTimer();
#endif

// You can use this to get the actual motor speed
// Do not set it, set targetPeriod instead / call eq_gotospeed()
unsigned long newPeriod = MAX_PERIOD_HZ; // period will be picked up next step

bool timer_running = false;

// Counts how many steps the motor did (clockwise => steps++; counterclockwise => steps--)
// Should not be set directly
// TODO: write function to reset it if required
long steps = 0;

unsigned long targetPeriod = MAX_PERIOD_HZ; // period the user wants

// Call dir_clockwise() / dir_counterclockwise() to set it
unsigned char direction = 0; // 0 = clockwise, 1 = counterclockwise

// Do not call directly
// set newPeriod according to targetPeriod
inline void _update_newPeriod() {
    // todo: limit acceleration
    if (direction == 0)
        steps ++;
    else
        steps --;

    // Proportional
    //float remaining = ((float)newPeriod - (float)targetPeriod)*(float)newPeriod*(float)newPeriod; // > 0 when acc.
    //newPeriod = (unsigned long)((float)newPeriod - remaining*STEPPER_PROP_ACCELL);

    // Constant acceleration
    long accel = newPeriod / STEPPER_CONST_ACCELL;
    if (newPeriod > accel + targetPeriod)
        newPeriod -= accel;
    else if (targetPeriod > accel + newPeriod)
        newPeriod += accel;
    else
        newPeriod = targetPeriod;
}

// Do not call directly
// Called when interrupt is raised by the timer
void _callback_timer() {
    // Do one step
    digitalWrite(STEPPER_PIN_STEP, HIGH);
    // delayMicroseconds(1) = 3.66us measured
    // delayMicroseconds(10) = 12.4us measured
    delayMicroseconds(10); // Wait long enough for the motor to pick it up
    digitalWrite(STEPPER_PIN_STEP, LOW);

    #ifdef CURIE
    CurieTimerOne.setPeriod(newPeriod);
    #endif
    #ifdef TIMERONE
    Timer1.setPeriod(newPeriod);
    #endif
    _update_newPeriod();
}

// Set the motor to turn clockwise
// Stop the motor before calling this function
void dir_clockwise() {
    // TODO: check if the motor is turning ?
    digitalWrite(STEPPER_PIN_DIR, HIGH);
    direction = 0;
}

// Set the motor to turn counterclockwise
// Stop the motor before calling this function
void dir_counterclockwise() {
    // TODO: check if the motor is turning ?
    digitalWrite(STEPPER_PIN_DIR, LOW);
    direction = 1;
}

// Call it to setup outputs and timer
void eq_setup() {
    // Motor's STEP pin, toogled by timer
    pinMode(STEPPER_PIN_STEP, OUTPUT);
    digitalWrite(STEPPER_PIN_STEP, LOW);

    #ifdef CURIE
        CurieTimerOne.start(MAX_PERIOD_HZ, &_callback_timer);
        CurieTimerOne.stop();
    #endif
    #ifdef TIMERONE
        Timer1.initialize(MAX_PERIOD_HZ); // microsecond
        // Timer1.attachInterrupt(_callback_timer); // Will be attached later
        // TODO: noInterrupts(); interrupts();
    #endif
    timer_running = false;

    pinMode(STEPPER_PIN_DIR, OUTPUT);
    dir_clockwise();

    pinMode(STEPPER_PIN_DISABLE, OUTPUT);
    digitalWrite(STEPPER_PIN_DISABLE, HIGH); // Do not power motor

    pinMode(STEPPER_PIN_MICRO, OUTPUT);
    digitalWrite(STEPPER_PIN_MICRO, HIGH); // x32 micro steps
    // TODO: write a function to set/unset microstepping
    //digitalWrite(STEPPER_PIN_MICRO, LOW); // Full steps
}

// Start the motor or change its speed if it is already running
void eq_gotospeed(unsigned long period) {
    digitalWrite(STEPPER_PIN_DISABLE, LOW); // Power motor
    targetPeriod = period;
    if (!timer_running) {
        timer_running = true;
        // setPeriod(1000): 1.002ms measured
        // Start from 0 rpm: choose a big enough period
        newPeriod = STEPPER_PERIOD_MIN;
        
        #ifdef CURIE
            CurieTimerOne.setPeriod(newPeriod);
            CurieTimerOne.start();
        #endif

        #ifdef TIMERONE
            //Timer1.setPeriod(newPeriod);
            //Timer1.detachInterrupt(); // stop
            Timer1.attachInterrupt(_callback_timer, newPeriod);
        #endif
    }
}

// Convert motor speed (tour per minute) to delay (micro second per (micro)step)
// 1tr per 10min = 21'583 us_per_step = 674 us_per_ustep
// 1tr per 23 h 56 min 4,09 s = 3'099'428 us_per_step = 96'857 us_per_ustep 
//#define TR_MIN_TO_DELAY(X) 1000000.0/(200.0*139.0*X/60.0)
#define TR_MIN_TO_DELAY(X) 1000000.0/(200.0*3.7*32*X/60.0) // 1:3.7 gearbox, x32 microstepping

// Stop motor and wait for it to stop (blocking)
void eq_stop_sync() {
    targetPeriod = STEPPER_PERIOD_MIN;
    while(newPeriod < STEPPER_PERIOD_MIN) // wait for the motor to be slow enough
        delayMicroseconds(STEPPER_PERIOD_MIN);
    digitalWrite(STEPPER_PIN_DISABLE, HIGH); // Power off
    timer_running = false;
    newPeriod = MAX_PERIOD_HZ;

    #ifdef CURIE
        CurieTimerOne.stop();
    #endif
    #ifdef TIMERONE
        Timer1.detachInterrupt();
    #endif
}

// Stop motor (set a very slow new speed)
// Returns immediatly, you can then use eq_stop_done() to check if it stopped
void eq_stop_async() {
    targetPeriod = STEPPER_PERIOD_MIN;
}

// Returns true if the motor is no longer running (cf eq_stop_async())
bool eq_stop_done() {
    if (!timer_running)
        return true;
    if (newPeriod >= STEPPER_PERIOD_MIN*0.7) {
        digitalWrite(STEPPER_PIN_DISABLE, HIGH); // Power off
        timer_running = false;
        newPeriod = MAX_PERIOD_HZ;
        #ifdef CURIE
            CurieTimerOne.stop();
        #endif
        #ifdef TIMERONE
            Timer1.detachInterrupt();
        #endif
        return true;
    }
    return false;
}

#endif // EQMOUNT_HPP