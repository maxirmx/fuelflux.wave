
# Orange Pi Zero 2W Square Signal Generator

This project provides three implementations of a square signal generator for Orange Pi Zero 2W (H618).

Default output pin: physical pin 16 (PWM4, GPIO line offset 9)  
Ground: physical pin 14  
Signal level: 3.3V

Frequency range: 1 Hz – 1 kHz (or higher depending on method)

## Orange Pi Zero 2W GPIO Offsets

PWM channels on `/dev/gpiochip0` (line offsets):

| PWM channel | Line offset | Physical pin |
|-------------|-------------|--------------|
| PWM1        | 21          | —            |
| PWM2        | 22          | —            |
| PWM3        | 2           | —            |
| PWM4        | 9           | Pin 16       |

Sysfs PWM channels on `/sys/class/pwm/pwmchip0`:

| Channel | PWM    | Physical pin |
|---------|--------|--------------|
| 0       | PWM1   | —            |
| 1       | PWM2   | —            |
| 2       | PWM3   | —            |
| 3       | PWM4   | Pin 16       |

## Dependencies

Install `libgpiod-dev` (required for implementations 1 and 2):

```
sudo apt-get install libgpiod-dev
```

## Build (CMake)

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Implementations

### 1. GPIO Software Toggle (`sqgen_gpio`)
Simple GPIO toggling loop using libgpiod with configurable chip, line offset, and frequency.

```
sudo ./build/sqgen_gpio --freq=10 --pin=9
sudo ./build/sqgen_gpio --freq=10 --pin=9 --chip=/dev/gpiochip0
sudo ./build/sqgen_gpio --help
```

Options:
- `--pin=N`    GPIO line offset (default: 9, i.e. PWM4 / physical pin 16)
- `--freq=HZ`  frequency in Hz (default: 1.0)
- `--chip=PATH` GPIO chip device (default: /dev/gpiochip0)
- `--help`     show help and exit

### 2. Software PWM via libgpiod (`sqgen_wiringop_pwm`)
Software PWM on any GPIO line with configurable frequency and duty cycle.  
Uses libgpiod for portable GPIO access.

```
sudo ./build/sqgen_wiringop_pwm --pin=9 --freq=1000 --duty=0.5
sudo ./build/sqgen_wiringop_pwm --help
```

Options:
- `--pin=N`    GPIO line offset (default: 9, i.e. PWM4 / physical pin 16)
- `--freq=HZ`  frequency in Hz (default: 1000)
- `--duty=D`   duty cycle 0..1 exclusive (default: 0.5)
- `--chip=PATH` GPIO chip device (default: /dev/gpiochip0)
- `--help`     show help and exit

### 3. Kernel PWM (sysfs) – Recommended (`sqgen_sysfs_pwm`)
Uses the kernel PWM subsystem via sysfs. Supports true 1 Hz and very low CPU usage.  
No external library required.

```
sudo ./build/sqgen_sysfs_pwm --freq=1 --duty=0.5
sudo ./build/sqgen_sysfs_pwm --help
```

Options:
- `--freq=HZ`      frequency in Hz (default: 1.0)
- `--duty=D`       duty cycle 0..1 (default: 0.5)
- `--chip=PATH`    sysfs PWM chip path (default: /sys/class/pwm/pwmchip0)
- `--channel=N`    PWM channel number (default: 3, i.e. PWM4 / physical pin 16)
- `--help`         show help and exit
