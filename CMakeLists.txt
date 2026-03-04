
cmake_minimum_required(VERSION 3.10)
project(sqgen)

set(CMAKE_CXX_STANDARD 17)

add_executable(sqgen_gpio sqgen_gpio.cpp)
add_executable(sqgen_wiringop_pwm sqgen_wiringop_pwm.cpp)
add_executable(sqgen_sysfs_pwm sqgen_sysfs_pwm.cpp)

target_link_libraries(sqgen_gpio gpiod)
target_link_libraries(sqgen_wiringop_pwm gpiod)
