// Program targetting Arduino Genuino 101
// x86 - Quark SE 32MHz

#include "eqmount.hpp"

#define mode0pin 7
#define mode1pin 8
#define mode2pin 9

#define encoderclkpin 11 // KY-040 encoder
#define encoredtpin 10 // KY-040 encoder

unsigned char encoderclk_last;
int encoderCounter = 0;

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("Eqmount 17th test");

    eq_setup();

    pinMode(mode0pin, INPUT_PULLUP);
    pinMode(mode1pin, INPUT_PULLUP);
    pinMode(mode2pin, INPUT_PULLUP);
    // digitalRead(mode0pin)
    // attachInterru pressed"pt(INT0, buttonPushed, FALLING);

    pinMode(encoderclkpin, INPUT_PULLUP); // PULLUP should be included on the board but test showed it doesn't work as expected
    pinMode(encoredtpin, INPUT_PULLUP);
    encoderclk_last = digitalRead(encoderclkpin);
}

void loop() {
    // Encoder
    delay(30);
    unsigned int encoderclk = digitalRead(encoderclkpin);
    unsigned int encoderdt = digitalRead(encoredtpin);
    if (encoderclk_last == HIGH && encoderclk == LOW) {
        if (encoderdt == HIGH) {
            encoderCounter++;
            Serial.print(encoderCounter);
            Serial.println(" Clockwise");
        } else {
            encoderCounter--;
            Serial.print(encoderCounter);
            Serial.println(" Counterclockwise");
        }
    }
    encoderclk_last = encoderclk;

    // TODO: GPS (get clock and position)

    // TODO: measure motor consumption with shunt resistor + AOP (x10, 1V = 1A)

    // Stepper motor
    /*
    Serial.println("Speeding up");
    eq_gotospeed(7000);
    // 1:3.7 4300 Full
    // 1:139 5000 Full
    while (steps < 200*139) {
        delay(500);
        if (!digitalRead(mode0pin)) // It is a pullup
        Serial.println("Mode 0");
        if (!digitalRead(mode1pin)) // It is a pullup
        Serial.println("Mode 1");
        if (!digitalRead(mode2pin)) // It is a pullup
        Serial.println("Mode 2");
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
    */
}