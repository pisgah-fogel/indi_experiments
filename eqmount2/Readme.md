This directory contains source code to control a arduino based motorisation system for my Equatorial mount

- Right ascension: NEMA17 motor (1:3.7 or 1:139 integrated gear box) fixed on mount's worm gear, 1 turn = ~10min
- No declinaison (Manual) because there is no enough room for me to put a motor on it

I am using an Arduino to control the system (easier to debug...) lateron I am thinking about a moving to an FPGA.

First experiements was using AccelStepper library, because it likes certain features I am writing my own.
- Using full steps to accelerate then switch to microstepping once the target speed is reached
- Fault detection (overcurrent or temperature)
- Clean emergency stop (decelerate)
- 