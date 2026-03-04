// sqgen_wiringop_pwm.cpp
// Hardware PWM generator via wiringOP (wiringPi-compatible API)
//
// Build:
//   g++ -O2 -std=c++17 sqgen_wiringop_pwm.cpp -o sqgen_wiringop_pwm -lwiringPi
//
// Run examples:
//   sudo ./sqgen_wiringop_pwm --pwm=4 --freq=1000 --duty=0.5
//
// Notes:
// - Requires PWM enabled in orangepi-config (System -> Hardware -> PWM...)
// - Uses wPi numbering mapping from the Orange Pi Zero 2W manual:
//     PWM1->wPi 21, PWM2->wPi 22, PWM3->wPi 2, PWM4->wPi 9
// - This implementation tries to approximate the requested frequency by selecting
//   pwmSetClock(div) and pwmSetRange(range). The achieved frequency is printed.
// - For *true* 1 Hz output, prefer the sysfs PWM version (sqgen_sysfs_pwm.cpp).

#include <wiringPi.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

static int pwm_channel_to_wpi(int pwm) {
  switch (pwm) {
    case 1: return 21; // PWM1
    case 2: return 22; // PWM2
    case 3: return 2;  // PWM3
    case 4: return 9;  // PWM4 (physical pin 16)
    default: return -1;
  }
}

static bool starts_with(const char* s, const char* pfx) {
  return std::strncmp(s, pfx, std::strlen(pfx)) == 0;
}

static void usage() {
  std::cout
    << "Usage: sudo ./sqgen_wiringop_pwm [--pwm=1..4] --freq=HZ [--duty=0..1]\n"
       "Defaults: --pwm=4 --freq=1000 --duty=0.5\n"
       "Example:  sudo ./sqgen_wiringop_pwm --pwm=4 --freq=1000 --duty=0.5\n";
}

int main(int argc, char** argv) {
  int pwm = 4;              // PWM4 -> physical pin 16
  double freq_hz = 1000.0;  // target Hz
  double duty = 0.5;        // 0..1

  for (int i = 1; i < argc; ++i) {
    if (starts_with(argv[i], "--pwm=")) pwm = std::stoi(argv[i] + 6);
    else if (starts_with(argv[i], "--freq=")) freq_hz = std::stod(argv[i] + 7);
    else if (starts_with(argv[i], "--duty=")) duty = std::stod(argv[i] + 7);
    else if (std::strcmp(argv[i], "--help") == 0) { usage(); return 0; }
    else { std::cerr << "Unknown arg: " << argv[i] << "\n"; usage(); return 2; }
  }

  if (pwm < 1 || pwm > 4) { std::cerr << "Error: --pwm must be 1..4\n"; return 2; }
  if (freq_hz <= 0.0) { std::cerr << "Error: --freq must be > 0\n"; return 2; }
  if (duty <= 0.0 || duty >= 1.0) { std::cerr << "Error: --duty must be in (0,1)\n"; return 2; }

  if (wiringPiSetup() != 0) {
    std::cerr << "Error: wiringPiSetup() failed (run as root)\n";
    return 1;
  }

  const int wpi = pwm_channel_to_wpi(pwm);
  if (wpi < 0) { std::cerr << "Error: PWM mapping failed\n"; return 1; }

  // Put pin into hardware PWM mode
  pinMode(wpi, PWM_OUTPUT);

  // Derive base clock from Orange Pi manual example: ~23475 Hz at range=1024, duty=512
  // => f_base ≈ 23475 * 1024
  const double f_base = 23475.0 * 1024.0;

  int bestDiv = -1;
  int bestRange = -1;
  double bestErr = 1e300;

  // wiringPi/wiringOP uses pwmSetClock(div) where div is integer. For Orange Pi it's typically 1..256.
  // pwmSetRange(range) in 1..65536.
  for (int div = 1; div <= 256; ++div) {
    double range_f = f_base / (div * freq_hz);
    if (range_f < 1.0 || range_f > 65536.0) continue;

    int range = (int)std::llround(range_f);
    if (range < 1) range = 1;
    if (range > 65536) range = 65536;

    double f_ach = f_base / (div * range);
    double err = std::fabs(f_ach - freq_hz);
    if (err < bestErr) {
      bestErr = err;
      bestDiv = div;
      bestRange = range;
    }
  }

  if (bestDiv < 0 || bestRange < 0) {
    std::cerr << "Requested frequency not achievable with wiringOP PWM parameters.\n"
                 "Tip: use sqgen_sysfs_pwm for very low frequencies (e.g., 1 Hz).\n";
    return 3;
  }

  pwmSetMode(PWM_MODE_MS);  // stable duty cycle (mark/space)
  pwmSetClock(bestDiv);
  pwmSetRange(bestRange);

  int ccr = (int)std::llround(duty * bestRange);
  if (ccr < 1) ccr = 1;
  if (ccr >= bestRange) ccr = bestRange - 1;

  pwmWrite(wpi, ccr);

  const double f_ach = f_base / (bestDiv * bestRange);

  std::cerr << "wiringOP PWM running:\n"
            << "  PWM" << pwm << " (wPi " << wpi << ")\n"
            << "  target freq=" << freq_hz << " Hz\n"
            << "  achieved ~" << f_ach << " Hz\n"
            << "  duty=" << duty << " (ccr=" << ccr << "/" << bestRange << ")\n"
            << "  clockDiv=" << bestDiv << " range=" << bestRange << "\n"
            << "Ctrl+C to stop.\n";

  // Keep process alive to keep PWM active
  for (;;) delay(1000);
  return 0;
}
