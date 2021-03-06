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

long encoder_step = 1;

DisplaySSD1306_128x32_I2C display(-1, {-1, 0x3C, OLED_PIN_CLK, OLED_PIN_TX, 0}); // No reset required for my board

unsigned char encoderclk_last;
unsigned char mode = 0; // State machine's state
char buffer[10];
int slew_counter = 0;

void display_title_mode_0() {
    display.fill(0x0);
    display.printFixed(0,  0, "0 IDLE", STYLE_NORMAL);
}

void display_title_mode_1() {
    display.fill(0x0);
}

void display_title_mode_2() {
    display.fill(0x0);
    display.printFixed(0,  0, "2 SLEW", STYLE_NORMAL);
}

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

    display.begin();
    display.setFixedFont( ssd1306xled_font6x8 );

    display_title_mode_0();
    mode = 0;
}

void loop() {
    // Se buttons to change states (ie mode)
    if (!digitalRead(BUTTON_PIN_0) && mode != 0) // It is a pullup
    {
        mode = 0;
        display_title_mode_0();
        if (timer_running) {
            eq_stop_async();
            wait_motor_stop();
        }
    }
    if (!digitalRead(BUTTON_PIN_1) && mode != 1) // It is a pullup
    {
        mode = 1;
        display_title_mode_1();

        if (!dir_clockwise()) {
            display.printFixed(0,  0, "1 STOPPING...", STYLE_NORMAL); // Replace first line

            Serial.flush();
            // Wait for motor to stop if it is running
            eq_stop_async();
            wait_motor_stop();
            dir_clockwise();
        }
        display.printFixed(0,  0, "1 SIDERAL    ", STYLE_NORMAL);
        delay(500); // Try to make sure we are going in the right direction
        eq_gotospeed(DEFAULT_SIDERAL_DELAY);
        display.printFixed(0,  3*8, "+ inf", STYLE_NORMAL);
        display.printFixed(6*8,  3*8, DEFAULT_SIDERAL_DELAY_STR, STYLE_NORMAL);
    }

    // In mode_1 use the rotary encoder to adjust the motor speed
    if (mode == 0) {
        bool serialAvailable = Serial.available() > 0;
        if (serialAvailable) {
            String tmp = Serial.readStringUntil('#');
            if (tmp.equals("CONNECT")) {
                mode = 1;
                display_title_mode_1();
                eq_gotospeed(DEFAULT_SIDERAL_DELAY);
                display.printFixed(0,  3*8, "+ inf", STYLE_NORMAL);
                display.printFixed(6*8,  3*8, DEFAULT_SIDERAL_DELAY_STR, STYLE_NORMAL);
                Serial.print("OK#");
            } else {
                Serial.flush();
            }
        }
    }
    else if (mode == 1) {
        if(!digitalRead(ENCODER_PIN_SW)) {
            if (encoder_step == 1) {
                encoder_step = 2;
            } else if (encoder_step == 2) {
                encoder_step = 5;
            } else if (encoder_step == 5) {
                encoder_step = 10;
            } else if (encoder_step == 10) {
                encoder_step = 100;
            } else if (encoder_step == 100) {
                encoder_step = 1000;
            } else {
                encoder_step = 1;
            }
            ltoa(encoder_step, buffer, 10);
            display.printFixed(0,  2*8, "x", STYLE_NORMAL);
            display.printFixed(1,  2*8, buffer, STYLE_NORMAL);
            delay(1000);
            display.printFixed(0,  2*8, "            ", STYLE_NORMAL);
        }

        // Encoder
        unsigned int encoderclk = digitalRead(ENCODER_PIN_CLK);
        unsigned int encoderdt = digitalRead(ENCODER_PIN_DT);
        bool serialAvailable = Serial.available() > 0;

        // Check for new motor speed sent via Serial connection
        // Or falling edge of encoderclk
        if (serialAvailable || (encoderclk_last == HIGH && encoderclk == LOW)) {
            if (serialAvailable) {
                //String tmp = Serial.readString();
                String tmp = Serial.readStringUntil('#');
                // Commands sent by Arduino-S (INDI) are:
                // DISCONNECT# - Disconnect
                // DEC+# - Guide north (DEC)
                // DEC-# - Guide south (DEC)
                // RA+# - Guide East (RA)
                // RA-# - Guide West (RA)
                // DEC0# - DEC axis back to normal speed
                // RA0# - RA axis back to normal speed
                if (tmp.equals("DISCONNECT")) {
                    //targetPeriod = DEFAULT_SIDERAL_DELAY;
                    mode = 0;
                    display_title_mode_0();
                    if (timer_running) {
                        eq_stop_async();
                        wait_motor_stop();
                    }
                    Serial.print("OK#");
                } else if (tmp.equals("RA0")) {
                    guiding = 0;
                    Serial.print("OK#");
                } else if (tmp.equals("RA+")) {
                    guiding -= encoder_step;
                    Serial.print("OK#");
                } else if (tmp.equals("RA-")) {
                    guiding += encoder_step;
                    Serial.print("OK#");
                } else {
                    Serial.flush();
                }
                
            } else {
                if (encoderdt == HIGH) {
                    targetPeriod -= encoder_step;
                } else {
                    targetPeriod += encoder_step;
                }
            }

            // Display
            ltoa(targetPeriod, buffer, 10);
            display.printFixed(6*8,  3*8, buffer, STYLE_NORMAL);
        }
        encoderclk_last = encoderclk;

        ltoa(newPeriod, buffer, 10);
        display.printFixed(0,  3*8, buffer, STYLE_NORMAL);
    }

    // TODO: GPS (get clock and position)

    // TODO: measure motor consumption with shunt resistor + AOP (x10, 1V = 1A)
}
