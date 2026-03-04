
#include <wiringPi.h>
#include <iostream>
#include <cstring>

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

    std::cout<<"PWM running"<<std::endl;

    while(true)
        delay(1000);
}
