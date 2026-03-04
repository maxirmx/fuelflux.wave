
#include <atomic>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstring>

static std::atomic<bool> running{true};

void stop(int){ running.store(false); }

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

static void usage() {
    std::cout
        << "Usage: ./sqgen_sysfs_pwm [--freq=HZ] [--duty=D] [--chip=PATH] [--channel=N]\n"
           "Defaults: --freq=1.0 --duty=0.5 --chip=/sys/class/pwm/pwmchip0 --channel=3\n"
           "Example:  sudo ./sqgen_sysfs_pwm --freq=1 --duty=0.5\n"
           "\n"
           "Orange Pi Zero 2W PWM sysfs channels (on pwmchip0):\n"
           "  channel 0=PWM1, 1=PWM2, 2=PWM3, 3=PWM4 (physical pin 16)\n"
           "\n"
           "Options:\n"
           "  --freq=HZ      frequency in Hz (default: 1.0)\n"
           "  --duty=D       duty cycle 0..1 (default: 0.5)\n"
           "  --chip=PATH    sysfs PWM chip path (default: /sys/class/pwm/pwmchip0)\n"
           "  --channel=N    PWM channel number (default: 3)\n"
           "  --help         show this help and exit\n";
}

int main(int argc,char**argv)
{
    double freq=1;
    double duty=0.5;
    std::string chip="/sys/class/pwm/pwmchip0";
    int channel=3;

    for(int i=1;i<argc;i++)
    {
        if(strncmp(argv[i],"--freq=",7)==0)
            freq=atof(argv[i]+7);
        else if(strncmp(argv[i],"--duty=",7)==0)
            duty=atof(argv[i]+7);
        else if(strncmp(argv[i],"--chip=",7)==0)
            chip=argv[i]+7;
        else if(strncmp(argv[i],"--channel=",10)==0)
            channel=atoi(argv[i]+10);
        else if(strcmp(argv[i],"--help")==0){ usage(); return 0; }
        else { std::cerr << "Unknown arg: " << argv[i] << "\n"; usage(); return 2; }
    }

    if(freq<=0.0){ std::cerr<<"Error: --freq must be > 0\n"; return 2; }
    if(duty<=0.0||duty>1.0){ std::cerr<<"Error: --duty must be in (0,1]\n"; return 2; }

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

    std::cerr << "Kernel PWM running:\n"
              << "  chip=" << chip << "  channel=" << channel << "\n"
              << "  freq=" << freq << " Hz  duty=" << duty << "\n"
              << "  period=" << period << " ns  duty_cycle=" << duty_cycle << " ns\n"
              << "Ctrl+C to stop.\n";

    signal(SIGINT,stop);

    while(running)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    write_file(pwm+"/enable","0");
}
