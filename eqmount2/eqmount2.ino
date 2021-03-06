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

#define xstr(a) str(a) // xstr(ENCODER_DEFAULT_VALUE) = "940"
#define str(a) #a // str(ENCODER_DEFAULT_VALUE) = "ENCODER_DEFAULT_VALUE"
#define ENCODER_DEFAULT_VALUE 940
#define ENCODER_DEFAULT_VALUE_STR xstr(ENCODER_DEFAULT_VALUE)

#define DEFAULT_SIDERAL_DELAY 26253 // Experimental: 94.5% of value given by TR_MIN_TO_DELAY(0.1)
// TODO: change with 26260 too slow/5min: trying 26255, little bit slow ? Bests results so far
// Trying 26250

//#define DEFAULT_SIDERAL_DELAY 25338 // Default value = TR_MIN_TO_DELAY(0.1)
#define DEFAULT_SIDERAL_DELAY_STR xstr(DEFAULT_SIDERAL_DELAY)
// My telescope requires 1 turn per 10min
// Max speed: 1:3.7 4300us/step Full
// Max speed: 1:139 5000us/step Full

#define SLEW_SPEED DEFAULT_SIDERAL_DELAY/7
#define SLEW_STEPS (200*3.7*2.4*2)

long encoder_step = 1;

DisplaySSD1306_128x32_I2C display(-1, {-1, 0x3C, OLED_PIN_CLK, OLED_PIN_TX, 0}); // No reset required for my board

// Font used for the OLED display
const PROGMEM uint8_t myfont []=
{
  0x00, 0x06, 0x08, 0x20,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sp
  0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, // !
  0x00, 0x00, 0x07, 0x00, 0x07, 0x00, // "
  0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14, // #
  0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12, // $
  0x00, 0x23, 0x13, 0x08, 0x64, 0x62, // %
  0x00, 0x36, 0x49, 0x55, 0x22, 0x50, // &
  0x00, 0x00, 0x05, 0x03, 0x00, 0x00, // '
  0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, // (
  0x00, 0x00, 0x41, 0x22, 0x1c, 0x00, // )
  0x00, 0x14, 0x08, 0x3E, 0x08, 0x14, // *
  0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, // +
  0x00, 0x00, 0x00, 0xA0, 0x60, 0x00, // ,
  0x00, 0x08, 0x08, 0x08, 0x08, 0x08, // -
  0x00, 0x00, 0x60, 0x60, 0x00, 0x00, // .
  0x00, 0x20, 0x10, 0x08, 0x04, 0x02, // /
  0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
  0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, // 1
  0x00, 0x42, 0x61, 0x51, 0x49, 0x46, // 2
  0x00, 0x21, 0x41, 0x45, 0x4B, 0x31, // 3
  0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, // 4
  0x00, 0x27, 0x45, 0x45, 0x45, 0x39, // 5
  0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
  0x00, 0x01, 0x71, 0x09, 0x05, 0x03, // 7
  0x00, 0x36, 0x49, 0x49, 0x49, 0x36, // 8
  0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, // 9
  0x00, 0x00, 0x36, 0x36, 0x00, 0x00, // :
  0x00, 0x00, 0x56, 0x36, 0x00, 0x00, // ;
  0x00, 0x08, 0x14, 0x22, 0x41, 0x00, // <
  0x00, 0x14, 0x14, 0x14, 0x14, 0x14, // =
  0x00, 0x00, 0x41, 0x22, 0x14, 0x08, // >
  0x00, 0x02, 0x01, 0x51, 0x09, 0x06, // ?
  0x00, 0x32, 0x49, 0x59, 0x51, 0x3E, // @
  0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C, // A
  0x00, 0x7F, 0x49, 0x49, 0x49, 0x36, // B
  0x00, 0x3E, 0x41, 0x41, 0x41, 0x22, // C
  0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C, // D
  0x00, 0x7F, 0x49, 0x49, 0x49, 0x41, // E
  0x00, 0x7F, 0x09, 0x09, 0x09, 0x01, // F
  0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A, // G
  0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F, // H
  0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, // I
  0x00, 0x20, 0x40, 0x41, 0x3F, 0x01, // J
  0x00, 0x7F, 0x08, 0x14, 0x22, 0x41, // K
  0x00, 0x7F, 0x40, 0x40, 0x40, 0x40, // L
  0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
  0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F, // N
  0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E, // O
  0x00, 0x7F, 0x09, 0x09, 0x09, 0x06, // P
  0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
  0x00, 0x7F, 0x09, 0x19, 0x29, 0x46, // R
  0x00, 0x46, 0x49, 0x49, 0x49, 0x31, // S
  0x00, 0x01, 0x01, 0x7F, 0x01, 0x01, // T
  0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F, // U
  0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F, // V
  0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F, // W
  0x00, 0x63, 0x14, 0x08, 0x14, 0x63, // X
  0x00, 0x07, 0x08, 0x70, 0x08, 0x07, // Y
  0x00, 0x61, 0x51, 0x49, 0x45, 0x43, // Z
  0x00, 0x00, 0x7F, 0x41, 0x41, 0x00, // [
  0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55, // 55
  0x00, 0x00, 0x41, 0x41, 0x7F, 0x00, // ]
  0x00, 0x04, 0x02, 0x01, 0x02, 0x04, // ^
  0x00, 0x40, 0x40, 0x40, 0x40, 0x40, // _
  0x00, 0x00, 0x01, 0x02, 0x04, 0x00, // '
  0x00, 0x20, 0x54, 0x54, 0x54, 0x78, // a
  0x00, 0x7F, 0x48, 0x44, 0x44, 0x38, // b
  0x00, 0x38, 0x44, 0x44, 0x44, 0x20, // c
  0x00, 0x38, 0x44, 0x44, 0x48, 0x7F, // d
  0x00, 0x38, 0x54, 0x54, 0x54, 0x18, // e
  0x00, 0x08, 0x7E, 0x09, 0x01, 0x02, // f
  0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C, // g
  0x00, 0x7F, 0x08, 0x04, 0x04, 0x78, // h
  0x00, 0x00, 0x44, 0x7D, 0x40, 0x00, // i
  0x00, 0x40, 0x80, 0x84, 0x7D, 0x00, // j
  0x00, 0x7F, 0x10, 0x28, 0x44, 0x00, // k
  0x00, 0x00, 0x41, 0x7F, 0x40, 0x00, // l
  0x00, 0x7C, 0x04, 0x18, 0x04, 0x78, // m
  0x00, 0x7C, 0x08, 0x04, 0x04, 0x78, // n
  0x00, 0x38, 0x44, 0x44, 0x44, 0x38, // o
  0x00, 0xFC, 0x24, 0x24, 0x24, 0x18, // p
  0x00, 0x18, 0x24, 0x24, 0x18, 0xFC, // q
  0x00, 0x7C, 0x08, 0x04, 0x04, 0x08, // r
  0x00, 0x48, 0x54, 0x54, 0x54, 0x20, // s
  0x00, 0x04, 0x3F, 0x44, 0x40, 0x20, // t
  0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C, // u
  0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C, // v
  0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C, // w
  0x00, 0x44, 0x28, 0x10, 0x28, 0x44, // x
  0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C, // y
  0x00, 0x44, 0x64, 0x54, 0x4C, 0x44, // z
  0x00, 0x00, 0x08, 0x77, 0x00, 0x00, // {
  0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, // |
  0x00, 0x00, 0x77, 0x08, 0x00, 0x00, // }
  0x00, 0x10, 0x08, 0x10, 0x08, 0x00, // ~
  0x14, 0x14, 0x14, 0x14, 0x14, 0x14, // horiz lines // DEL
  0x00
  /* This byte is required for italic type of font */
};

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
    Serial.begin(9600);
    while (!Serial);
    Serial.setTimeout(500);

    Serial.println("Eqmount2 V1 Exp.");

    eq_setup();

    pinMode(BUTTON_PIN_0, INPUT_PULLUP);
    pinMode(BUTTON_PIN_1, INPUT_PULLUP);
    pinMode(BUTTON_PIN_2, INPUT_PULLUP);
    pinMode(ENCODER_PIN_SW, INPUT_PULLUP);

    pinMode(ENCODER_PIN_CLK, INPUT_PULLUP); // PULLUP should be included on the board but test showed it doesn't work as expected
    pinMode(ENCODER_PIN_DT, INPUT_PULLUP);
    encoderclk_last = digitalRead(ENCODER_PIN_CLK);

    display.begin();
    display.setFixedFont( myfont );

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
    if (!digitalRead(BUTTON_PIN_2)  && mode != 2) // It is a pullup
    {
        mode = 2;
        display_title_mode_2();
        display.printFixed(0,  3*8, "0       ", STYLE_NORMAL);
    }

    // In mode_1 use the rotary encoder to adjust the motor speed
    if (mode == 0) {
        if (Serial.available() > 0) {
            // TODO: enter tracking
            Serial.flush();
        }
    }
    else if (mode == 1) {
        if(!digitalRead(ENCODER_PIN_SW)) {
            if (encoder_step == 1) {
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
                long tmp = Serial.parseInt(SKIP_ALL);
                targetPeriod += tmp;
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
    } else if (mode == 2)
    {
        // Encoder
        unsigned int encoderclk = digitalRead(ENCODER_PIN_CLK);
        unsigned int encoderdt = digitalRead(ENCODER_PIN_DT);
        bool serialAvailable = Serial.available() > 0;

        // Check for new motor speed sent via Serial connection
        // Or falling edge of encoderclk
        if (serialAvailable || (encoderclk_last == HIGH && encoderclk == LOW)) {
            if (encoderdt == HIGH) {
                slew_counter++;
            } else {
                slew_counter--;
            }
            itoa(slew_counter, buffer, 10);
            display.printFixed(0,  3*8, "        ", STYLE_NORMAL); // clear line
            display.printFixed(0,  3*8, buffer, STYLE_NORMAL);
        }
        encoderclk_last = encoderclk;

        if(!digitalRead(ENCODER_PIN_SW))
        {
            display.printFixed(0,  0, "Release button ", STYLE_NORMAL);
            delay(1000);
            if(slew_counter < 0) {
                if (!dir_counterclockwise()) {
                    display.printFixed(0,  0, "Changing direction ", STYLE_NORMAL);
                    eq_stop_async();
                    wait_motor_stop();
                }
                dir_counterclockwise();
                display.printFixed(0,  0, "Going back...     ", STYLE_NORMAL);
                eq_gotospeed(SLEW_SPEED);
                long target_steps = steps + slew_counter*SLEW_STEPS;
                while (steps > target_steps && digitalRead(ENCODER_PIN_SW)) { // switch encoder to abord
                    ltoa(steps-target_steps, buffer, 10);
                    display.printFixed(0,  3*8, buffer, STYLE_NORMAL);
                    delay(500);
                }
                display.printFixed(0,  0, "Done, now tracking", STYLE_NORMAL);
                eq_stop_async();
                wait_motor_stop();
                dir_clockwise(); // Avoid problem if other functions do not expect counterwise...
                delay(500); // Try to make sure we are going in the right direction
                dir_clockwise();
                delay(500);
                eq_gotospeed(DEFAULT_SIDERAL_DELAY); // Now tracking; TODO: Not working !!!
                display_title_mode_2();
            } else if (slew_counter > 0) {
                display.printFixed(0,  0, "Going forward...  ", STYLE_NORMAL);
                dir_clockwise();
                eq_gotospeed(SLEW_SPEED);
                long target_steps = steps + slew_counter*SLEW_STEPS;
                while (steps < target_steps && digitalRead(ENCODER_PIN_SW)) { // switch encoder to abord
                    ltoa(target_steps-steps, buffer, 10);
                    display.printFixed(0,  3*8, buffer, STYLE_NORMAL);
                    delay(500);
                }
                display.printFixed(0,  0, "Done, back to trck.", STYLE_NORMAL);
                //eq_stop_async();
                //wait_motor_stop();
                eq_gotospeed(DEFAULT_SIDERAL_DELAY); // Now tracking
                display_title_mode_2();
            }
        }
    }
    

    // TODO: GPS (get clock and position)

    // TODO: measure motor consumption with shunt resistor + AOP (x10, 1V = 1A)
}
