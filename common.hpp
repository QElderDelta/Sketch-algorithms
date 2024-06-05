#pragma once

#include <chrono>
#include <iostream>
#include <string>

struct Timer
{
    Timer(std::string aOperation);
    ~Timer();

private:
    std::string                                                 operation;
    std::chrono::time_point<std::chrono::high_resolution_clock> begin;
};