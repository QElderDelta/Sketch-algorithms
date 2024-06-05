#include "common.hpp"

Timer::Timer(std::string aOperation)
: operation(std::move(aOperation)), begin(std::chrono::high_resolution_clock::now())
{}

Timer::~Timer()
{
    std::cout << operation << " took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::high_resolution_clock::now() - begin)
                     .count()
              << "ms" << std::endl;
}