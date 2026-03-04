
#include <wiringPi.h>
#include <iostream>
#include <cstring>
#include <csignal>

static volatile bool running = true;

void stop(int){ running = false; }

int main(int argc,char**argv)
{
    double duty = 0.5;

    for(int i=1;i<argc;i++)
    {
        if(strncmp(argv[i],"--duty=",7)==0)
            duty = atof(argv[i]+7);
    }

    int pwmPin = 9;

    wiringPiSetup();
    pinMode(pwmPin,PWM_OUTPUT);

    int range = 1024;
    int divisor = 32;

    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(range);
    pwmSetClock(divisor);

    int value = range * duty;
    pwmWrite(pwmPin,value);

    signal(SIGINT, stop);
    signal(SIGTERM, stop);

    std::cout<<"PWM running"<<std::endl;

    while(running)
        delay(1000);
}
