
# Orange Pi Zero 2W Square Signal Generator

This project provides three implementations of a square signal generator for Orange Pi.

Default output pin: physical pin 16 (PWM4)  
Ground: physical pin 14  
Signal level: 3.3V

Frequency range: 1 Hz – 1 kHz (or higher depending on method)

## Implementations

### 1. GPIO Software Toggle
Simple GPIO toggling loop with configurable pin and frequency.

Build:
g++ -O2 -std=c++17 sqgen_gpio.cpp -o sqgen_gpio -lwiringPi

Run:
sudo ./sqgen_gpio --freq=10 --pin=11

### 2. Hardware PWM (wiringOP)
Uses hardware PWM controller.

Build:
g++ -O2 -std=c++17 sqgen_wiringop_pwm.cpp -o sqgen_wiringop_pwm -lwiringPi

Run:
sudo ./sqgen_wiringop_pwm --freq=1000

### 3. Kernel PWM (sysfs) – Recommended
Supports true 1 Hz and very low CPU usage.

Build:
g++ -O2 -std=c++17 sqgen_sysfs_pwm.cpp -o sqgen_pwm

Run:
sudo ./sqgen_pwm --freq=1
