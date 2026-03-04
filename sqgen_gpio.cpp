
#include <wiringPi.h>
#include <iostream>
#include <cstring>
#include <csignal>

volatile bool run = true;

void stop(int){ run = false; }

int main(int argc,char**argv)
{
    int pin = 11;
    double freq = 1.0;

    for(int i=1;i<argc;i++)
    {
        if(strncmp(argv[i],"--pin=",6)==0)
            pin = atoi(argv[i]+6);
        if(strncmp(argv[i],"--freq=",7)==0)
            freq = atof(argv[i]+7);
    }

    if(freq <= 0.0)
    {
        std::cerr << "Error: --freq must be > 0\n";
        return 2;
    }

    wiringPiSetupPhys();
    pinMode(pin,OUTPUT);

    double period = 1.0 / freq;
    int high_us = period*500000;
    int low_us = period*500000;

    signal(SIGINT,stop);

    while(run)
    {
        digitalWrite(pin,1);
        delayMicroseconds(high_us);

        digitalWrite(pin,0);
        delayMicroseconds(low_us);
    }
}
