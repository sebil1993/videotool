#include <stdio.h>
#include <iostream>
#include <thread>
#include <chrono>

void task1(int waitTime)
{
    std::cout << "started" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
    std::cout << "finished" << std::endl;
}
int main()
{
    std::cout << "started" << std::endl;
    std::thread t1(task1, 2000);
    t1.join();
}