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


#define STEPPER_PIN_STEP 3
#define STEPPER_PIN_DIR 5
#define STEPPER_PIN_DISABLE 6
#define STEPPER_PIN_MICRO 4

#define STEPPER_PERIOD_MIN 100000 // experimental value
#define STEPPER_PROP_ACCELL 0.00000000001 // experimental value

#ifdef CURIE
    CurieTimer timer = CurieTimer();
#endif
unsigned long newPeriod = MAX_PERIOD_HZ; // period will be picked up next step

bool timer_running = false;

unsigned long steps = 0;

unsigned long targetPeriod = MAX_PERIOD_HZ; // speed the user wants

// set newPeriod according to targetPeriod
inline void _update_newPeriod() {
    // Proportional
    // todo: limit acceleration
    steps ++;
    float remaining = ((float)newPeriod - (float)targetPeriod)*(float)newPeriod*(float)newPeriod; // > 0 when acc.
    newPeriod = (unsigned long)((float)newPeriod - remaining*STEPPER_PROP_ACCELL);
}

void _callback_timer() {
    // Do one step
    digitalWrite(STEPPER_PIN_STEP, HIGH);
    delayMicroseconds(10);
    // delayMicroseconds(1) = 3.66us
    // delayMicroseconds(10) = 12.4us
    digitalWrite(STEPPER_PIN_STEP, LOW);

    #ifdef CURIE
    CurieTimerOne.setPeriod(newPeriod);
    #endif
    #ifdef TIMERONE
    Timer1.setPeriod(newPeriod);
    #endif
    _update_newPeriod();
}

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
    digitalWrite(STEPPER_PIN_DIR, HIGH); // clockwise

    pinMode(STEPPER_PIN_DISABLE, OUTPUT);
    digitalWrite(STEPPER_PIN_DISABLE, HIGH); // Do not power motor

    pinMode(STEPPER_PIN_MICRO, OUTPUT);
    //digitalWrite(STEPPER_PIN_MICRO, HIGH); // x32 micro steps
    digitalWrite(STEPPER_PIN_MICRO, LOW); // Full steps
}

void eq_gotospeed(unsigned long period) {
    digitalWrite(STEPPER_PIN_DISABLE, LOW); // Power motor
    targetPeriod = period;
    if (!timer_running) {
        timer_running = true;
        // setPeriod(1000): 1.002ms
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

inline void eq_gotospeed_tr_per_min(float tr_per_min) {
    float us_per_step = 1000000.0/(200.0*139.0*tr_per_min/60.0);
    // 1tr per 10min = 21'583 us_per_step = 674 us_per_ustep
    // 1tr per 23 h 56 min 4,09 s = 3'099'428 us_per_step = 96'857 us_per_ustep 
    eq_gotospeed((unsigned long)us_per_step);
}

void eq_stop_sync() {
    targetPeriod = STEPPER_PERIOD_MIN;
    while(newPeriod < STEPPER_PERIOD_MIN)
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

void eq_stop_async() {
    targetPeriod = STEPPER_PERIOD_MIN;
}

bool eq_stop_done() {
    if (!timer_running)
        return true;
    if (newPeriod >= STEPPER_PERIOD_MIN*0.9) {
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