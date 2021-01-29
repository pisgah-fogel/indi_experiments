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
    Serial.println("Speeding up");
    eq_gotospeed(10000); // 15000 ? max full step ?
    for (int i = 0; i < 20; i++) {
        delay(1000);
        Serial.print("dec ");
        Serial.println(newPeriod);
    }
    Serial.println("Stopping");
    eq_stop_async();
    while (!eq_stop_done()) {
        delay(1000);
        Serial.print("inc ");
        Serial.println(newPeriod);
    }
    delay(2000);
}