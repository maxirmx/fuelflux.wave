// sqgen_wiringop_pwm.cpp
// Software PWM generator via libgpiod
//
// Build:
//   g++ -O2 -std=c++17 sqgen_wiringop_pwm.cpp -o sqgen_wiringop_pwm -lgpiod
//
// Run examples:
//   sudo ./sqgen_wiringop_pwm --pin=270 --freq=1000 --duty=0.5
//
// Notes:
// - Uses GPIO line offset numbers (not physical pin numbers, not WiringPi wPi numbers).
// - Orange Pi Zero 2W PWM channel GPIO offsets:
//     PWM1->GPIO 267 (pin 32), PWM2->GPIO 268 (pin 33),
//     PWM3->GPIO 269 (pin 7),  PWM4->GPIO 270 (pin 16)
// - For kernel-managed PWM with very low CPU usage, prefer sqgen_sysfs_pwm.

#include <gpiod.h>
#include <atomic>
#include <cstring>
#include <csignal>
#include <iostream>
#include <string>
#include <unistd.h>

static std::atomic<bool> running{true};

static void stop(int){ running.store(false); }

static bool starts_with(const char* s, const char* pfx) {
  return std::strncmp(s, pfx, std::strlen(pfx)) == 0;
}

static void usage() {
  std::cout
    << "Usage: ./sqgen_wiringop_pwm [--pin=N] [--freq=HZ] [--duty=0..1] [--chip=PATH]\n"
       "Defaults: --pin=270 --freq=1000 --duty=0.5 --chip=/dev/gpiochip0\n"
       "Example:  sudo ./sqgen_wiringop_pwm --pin=270 --freq=1000 --duty=0.5\n"
       "\n"
       "Orange Pi Zero 2W GPIO line offsets (use these as --pin values):\n"
       "  PWM1->GPIO 267 (pin 32), PWM2->GPIO 268 (pin 33),\n"
       "  PWM3->GPIO 269 (pin 7),  PWM4->GPIO 270 (pin 16)\n";
}

int main(int argc, char** argv) {
  int pin = 270; // PWM4 on Orange Pi Zero 2W, GPIO 270, physical pin 16
  double freq_hz = 1000.0;
  double duty = 0.5;
  const char* chip_path = "/dev/gpiochip0";

  for (int i = 1; i < argc; ++i) {
    if (starts_with(argv[i], "--pin=")) pin = std::stoi(argv[i] + 6);
    else if (starts_with(argv[i], "--freq=")) freq_hz = std::stod(argv[i] + 7);
    else if (starts_with(argv[i], "--duty=")) duty = std::stod(argv[i] + 7);
    else if (starts_with(argv[i], "--chip=")) chip_path = argv[i] + 7;
    else if (std::strcmp(argv[i], "--help") == 0) { usage(); return 0; }
    else { std::cerr << "Unknown arg: " << argv[i] << "\n"; usage(); return 2; }
  }

  if (freq_hz <= 0.0) { std::cerr << "Error: --freq must be > 0\n"; return 2; }
  if (duty <= 0.0 || duty >= 1.0) { std::cerr << "Error: --duty must be in (0,1)\n"; return 2; }

  struct gpiod_chip* chip = gpiod_chip_open(chip_path);
  if (!chip) {
    std::cerr << "Error: cannot open " << chip_path << "\n";
    return 1;
  }

  struct gpiod_line* line = gpiod_chip_get_line(chip, (unsigned int)pin);
  if (!line) {
    std::cerr << "Error: cannot get line " << pin << "\n";
    gpiod_chip_close(chip);
    return 1;
  }

  if (gpiod_line_request_output(line, "sqgen_wiringop_pwm", 0) < 0) {
    std::cerr << "Error: cannot request GPIO line " << pin << " as output\n";
    gpiod_chip_close(chip);
    return 1;
  }

  double period = 1.0 / freq_hz;
  long high_us = (long)(period * duty * 1e6);
  long low_us  = (long)(period * (1.0 - duty) * 1e6);
  if (high_us < 1 || low_us < 1) {
    std::cerr << "Error: frequency/duty combination produces sub-microsecond timing; "
                 "reduce --freq or adjust --duty\n";
    return 2;
  }

  std::cerr << "Software PWM running:\n"
            << "  chip=" << chip_path << "  line=" << pin << "\n"
            << "  freq=" << freq_hz << " Hz  duty=" << duty << "\n"
            << "  high=" << high_us << " us  low=" << low_us << " us\n"
            << "Ctrl+C to stop.\n";

  signal(SIGINT, stop);

  while (running) {
    gpiod_line_set_value(line, 1);
    usleep((useconds_t)high_us);
    gpiod_line_set_value(line, 0);
    usleep((useconds_t)low_us);
  }

  gpiod_line_release(line);
  gpiod_chip_close(chip);
  return 0;
}
