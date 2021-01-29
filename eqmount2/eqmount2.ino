// Program targetting Arduino Genuino 101
// x86 - Quark SE 32MHz

#include "eqmount.hpp"

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("Eqmount first test");

    eq_setup();
}

void loop() {
    eq_gotospeed(10000);
    delay(1000);
    eq_gotospeed(5000);
    delay(1000);
    eq_gotospeed(10000);
    delay(1000);
}