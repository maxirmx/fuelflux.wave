
#include <gpiod.h>
#include <iostream>
#include <cstring>
#include <csignal>
#include <unistd.h>

volatile bool run = true;

void stop(int){ run = false; }

static void usage() {
    std::cout
        << "Usage: ./sqgen_gpio [--pin=N] [--freq=HZ] [--chip=PATH]\n"
           "Defaults: --pin=270 --freq=1.0 --chip=/dev/gpiochip0\n"
           "Example:  sudo ./sqgen_gpio --pin=270 --freq=10\n"
           "\n"
           "Orange Pi Zero 2W GPIO line offsets (use these as --pin values):\n"
           "  PWM1->GPIO 267 (pin 32), PWM2->GPIO 268 (pin 33),\n"
           "  PWM3->GPIO 269 (pin 7),  PWM4->GPIO 270 (pin 16)\n"
           "\n"
           "Options:\n"
           "  --pin=N     GPIO line offset (default: 270, i.e. PWM4 / physical pin 16)\n"
           "  --freq=HZ   frequency in Hz (default: 1.0)\n"
           "  --chip=PATH GPIO chip device (default: /dev/gpiochip0)\n"
           "  --help      show this help and exit\n";
}

int main(int argc,char**argv)
{
    int pin = 270; // PWM4 on Orange Pi Zero 2W, GPIO 270, physical pin 16
    double freq = 1.0;
    const char* chip_path = "/dev/gpiochip0";

    for(int i=1;i<argc;i++)
    {
        if(strncmp(argv[i],"--pin=",6)==0)
            pin = atoi(argv[i]+6);
        else if(strncmp(argv[i],"--freq=",7)==0)
            freq = atof(argv[i]+7);
        else if(strncmp(argv[i],"--chip=",7)==0)
            chip_path = argv[i]+7;
        else if(strcmp(argv[i],"--help")==0){ usage(); return 0; }
        else { std::cerr << "Unknown arg: " << argv[i] << "\n"; usage(); return 2; }
    }

    if(freq <= 0.0)
    {
        std::cerr << "Error: --freq must be > 0\n";
        return 2;
    }

    struct gpiod_chip* chip = gpiod_chip_open(chip_path);
    if(!chip)
    {
        std::cerr << "Error: cannot open " << chip_path << "\n";
        return 1;
    }

    struct gpiod_line* line = gpiod_chip_get_line(chip, (unsigned int)pin);
    if(!line)
    {
        std::cerr << "Error: cannot get line " << pin << "\n";
        gpiod_chip_close(chip);
        return 1;
    }

    if(gpiod_line_request_output(line, "sqgen_gpio", 0) < 0)
    {
        std::cerr << "Error: cannot request line as output\n";
        gpiod_chip_close(chip);
        return 1;
    }

    double period = 1.0 / freq;
    int high_us = (int)(period * 500000);
    int low_us  = (int)(period * 500000);

    std::cerr << "GPIO square wave running:\n"
              << "  chip=" << chip_path << "  line=" << pin << "\n"
              << "  freq=" << freq << " Hz  duty=0.5\n"
              << "  high=" << high_us << " us  low=" << low_us << " us\n"
              << "Ctrl+C to stop.\n";

    signal(SIGINT,stop);

    while(run)
    {
        gpiod_line_set_value(line, 1);
        usleep((useconds_t)high_us);

        gpiod_line_set_value(line, 0);
        usleep((useconds_t)low_us);
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}
