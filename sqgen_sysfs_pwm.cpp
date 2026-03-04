
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstring>

static volatile bool running=true;

void stop(int){ running=false; }

bool write_file(const std::string &p,const std::string &v)
{
    std::ofstream f(p);
    if(!f) return false;
    f<<v;
    return true;
}

bool exists(const std::string &p)
{
    std::ifstream f(p);
    return f.good();
}

int main(int argc,char**argv)
{
    double freq=1;
    double duty=0.5;

    for(int i=1;i<argc;i++)
    {
        if(strncmp(argv[i],"--freq=",7)==0)
            freq=atof(argv[i]+7);

        if(strncmp(argv[i],"--duty=",7)==0)
            duty=atof(argv[i]+7);
    }

    std::string chip="/sys/class/pwm/pwmchip0";
    int channel=3;

    std::string pwm=chip+"/pwm"+std::to_string(channel);

    if(!exists(pwm))
    {
        write_file(chip+"/export",std::to_string(channel));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    uint64_t period=(uint64_t)(1e9/freq);
    uint64_t duty_cycle=(uint64_t)(period*duty);

    write_file(pwm+"/enable","0");
    write_file(pwm+"/period",std::to_string(period));
    write_file(pwm+"/duty_cycle",std::to_string(duty_cycle));
    write_file(pwm+"/enable","1");

    std::cout<<"PWM started on pin16"<<std::endl;

    signal(SIGINT,stop);

    while(running)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    write_file(pwm+"/enable","0");
}
