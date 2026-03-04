
#include <gpiod.h>
#include <iostream>
#include <cstring>
#include <csignal>
#include <unistd.h>

volatile bool run = true;

void stop(int){ run = false; }

int main(int argc,char**argv)
{
    int pin = 11;
    double freq = 1.0;
    const char* chip_path = "/dev/gpiochip0";

    for(int i=1;i<argc;i++)
    {
        if(strncmp(argv[i],"--pin=",6)==0)
            pin = atoi(argv[i]+6);
        if(strncmp(argv[i],"--freq=",7)==0)
            freq = atof(argv[i]+7);
        if(strncmp(argv[i],"--chip=",7)==0)
            chip_path = argv[i]+7;
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
