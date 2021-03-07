// Program targetting
// Arduino Genuino 101 / x86 - Quark SE 32MHz
// Arduino Nano V3 / Atmega168

#include "lcdgfx.h"

#define BUTTON_PIN_0 10
#define BUTTON_PIN_1 9
#define BUTTON_PIN_2 8

#define STEPPER_PIN_STEP 3
#define STEPPER_PIN_DIR 5
#define STEPPER_PIN_DISABLE 6
#define STEPPER_PIN_MICRO 4

#define ENCODER_PIN_CLK 12 // KY-040 encoder
#define ENCODER_PIN_DT 11 // KY-040 encoder
#define ENCODER_PIN_SW 13

#include "eqmount.hpp"

#ifndef __AVR_ATmega168__
#warning "OLED pinout may not be correct on your board (23(I2C TX) 24(I2C CLK))"
#endif
// Arduino Nano Atmega168: 24-23
// Arduino Pro mini: 28-27 ? In arduino choose model "Arduino Pro", Atmega328P (3.3V, 8MHz), Arduino as ISCP and Program using programmer
#define OLED_PIN_CLK 24 // Pin for I2C communication (you can write/use software I2C if you want)
#define OLED_PIN_TX 23 // cf OLED_PIN_CLK

#define xstr(a) str(a)
#define str(a) #a
#define DEFAULT_SIDERAL_DELAY 25270 // In theory should be: TR_MIN_TO_DELAY(0.1) = 25270
// Experiments: 26260 too slow/5min
// 26255, little bit slow ? Bests results so far

#define DEFAULT_SIDERAL_DELAY_STR xstr(DEFAULT_SIDERAL_DELAY)
// My telescope requires 1 turn per 10min
// Max speed: 1:3.7 4300us/step Full
// Max speed: 1:139 5000us/step Full

#define SLEW_SPEED DEFAULT_SIDERAL_DELAY/7
#define SLEW_STEPS (200*3.7*2.4*2)

long guiding_steps = 10;

DisplaySSD1306_128x32_I2C display(-1, {-1, 0x3C, OLED_PIN_CLK, OLED_PIN_TX, 0}); // No reset required for my board

unsigned char encoderclk_last;
char buffer[10];

void wait_motor_stop() {
    while (!eq_stop_done()) {
        ltoa(newPeriod, buffer, 10);
        display.printFixed(0,  3*8, buffer, STYLE_NORMAL); // Print progress
        delay(1000);
    }
    display.printFixed(0,  3*8, "STOPPED    ", STYLE_NORMAL); // Clear line
    delay(1000);
    display.printFixed(0,  3*8, "           ", STYLE_NORMAL); // Clear line
}

void setup() {
    Serial.begin(57600, SERIAL_8N1);

    display.begin();
    display.setFixedFont( ssd1306xled_font6x8 );
    display.fill(0x0);
    display.printFixed(0,  0, "REMOTE     ", STYLE_NORMAL);

    while (!Serial);
    Serial.setTimeout(500);
    Serial.println("INITIALIZED#");

    //Serial.println("Eqmount2 V1 Exp.");

    eq_setup();

    pinMode(BUTTON_PIN_0, INPUT_PULLUP);
    pinMode(BUTTON_PIN_1, INPUT_PULLUP);
    pinMode(BUTTON_PIN_2, INPUT_PULLUP);
    pinMode(ENCODER_PIN_SW, INPUT_PULLUP);

    pinMode(ENCODER_PIN_CLK, INPUT_PULLUP); // PULLUP should be included on the board but test showed it doesn't work as expected
    pinMode(ENCODER_PIN_DT, INPUT_PULLUP);
    encoderclk_last = digitalRead(ENCODER_PIN_CLK);
}

void loop() {

    // Serial interface
    bool serialAvailable = Serial.available() > 0;
    if (serialAvailable) {
        //String tmp = Serial.readString();
        String tmp = Serial.readStringUntil('#');
        // Commands sent by Arduino-S (INDI) are:
        // CONNECT# - Connect and start the motor - Returns OK#
        // DISCONNECT# - Disconnect and turn off - Returns OK#
        // DEC+# - Guide north (DEC) - Returns OK# if implemented
        // DEC-# - Guide south (DEC) - Returns OK# if implemented
        // RA+# - Guide East (RA) - Returns OK#
        // RA-# - Guide West (RA) - Returns OK#
        // DEC0# - DEC axis back to normal speed - Returns OK# if implemented
        // RA0# - RA axis back to normal speed - Returns OK#
        // SET#<Double value># - Set the motor s speed - Returns OK#
        // READ# - Read the motor s speed - Returns double
        if (tmp.equals("CONNECT")) {
                guiding = 0;
                display.printFixed(0,  0, "CONNECTED   ", STYLE_NORMAL);
                dir_clockwise();
                eq_gotospeed(DEFAULT_SIDERAL_DELAY);
                Serial.print("OK#");
        } else if (tmp.equals("DISCONNECT")) {
            guiding = 0;
            display.printFixed(0,  0, "DISCONNECTED", STYLE_NORMAL);
            //targetPeriod = DEFAULT_SIDERAL_DELAY;
            if (timer_running) {
                eq_stop_async();
                wait_motor_stop();
            }
            Serial.print("OK#");
            display.printFixed(0,  0, "WAIT FOR CO.", STYLE_NORMAL);
        } else if (tmp.equals("RA0")) {
            guiding = 0;
            Serial.print("OK#");
        } else if (tmp.equals("RA+")) {
            guiding -= guiding_steps;
            Serial.print("OK#");
        } else if (tmp.equals("RA-")) {
            guiding += guiding_steps;
            Serial.print("OK#");
        }
        else if (tmp.equals("SET")) {
            guiding = 0;
            //int value = Serial.parseInt(SKIP_NONE, "#");targetPeriod = value; // Max value: 16bits = 65536
            String str_value = Serial.readStringUntil('#');
            double value = str_value.toDouble();
            if (value > 0) {
                targetPeriod = value;
                Serial.print("OK#");
            }
        }
        else if (tmp.equals("READ")) {
            if (timer_running) {
                Serial.print(newPeriod);
            }
            else {
                Serial.print("0");
            }
            Serial.print("#");
        }
        Serial.flush();
    }

    // Encoder's button
    if(!digitalRead(ENCODER_PIN_SW)) {
        if (guiding_steps == 1) {
            guiding_steps = 2;
        } else if (guiding_steps == 2) {
            guiding_steps = 5;
        } else if (guiding_steps == 5) {
            guiding_steps = 10;
        } else if (guiding_steps == 10) {
            guiding_steps = 100;
        } else if (guiding_steps == 100) {
            guiding_steps = 1000;
        } else {
            guiding_steps = 1;
        }
        //ltoa(guiding_steps, buffer, 10);
        sprintf(buffer, "%5d", guiding_steps);
        display.printFixed(0,  2*8, "x", STYLE_NORMAL);
        display.printFixed(1,  2*8, buffer, STYLE_NORMAL);
        delay(1000);
        display.printFixed(0,  2*8, "            ", STYLE_NORMAL);
    }

    // Read Motor's speed
    //ltoa(targetPeriod, buffer, 10);
    sprintf(buffer, "%6u", targetPeriod);
    display.printFixed(6*8,  3*8, buffer, STYLE_NORMAL);
    //ltoa(newPeriod, buffer, 10);
    sprintf(buffer, "%6u", newPeriod);
    display.printFixed(0,  3*8, buffer, STYLE_NORMAL);
}
