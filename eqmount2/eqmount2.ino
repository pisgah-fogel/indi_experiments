// Program targetting Arduino Genuino 101
// x86 - Quark SE 32MHz

#include "eqmount.hpp"

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("Eqmount 17th test");

    eq_setup();
}

void loop() {
    Serial.println("Speeding up");
    eq_gotospeed(7000);
    // 1:3.7 4300 Full
    // 1:139 5000 Full
    while (steps < 200*139) {
        delay(1000);
        Serial.print("dec ");
        Serial.println(newPeriod);
    }
    steps = 0;
    Serial.println("Stopping");
    eq_stop_async();
    while (!eq_stop_done()) {
        delay(1000);
        Serial.print("inc ");
        Serial.println(newPeriod);
    }
    delay(2000);
}