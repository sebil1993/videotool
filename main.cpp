#include "Onvif/Onvif.h"
#include <stdio.h>  /* printf */
#include <time.h>   /* time_t, struct tm, difftime, time, mktime */
#include <iostream> // std::cout, std::endl
#include <thread>   // std::this_thread::sleep_for
#include <chrono>   // std::chrono::seconds




int main()
{
    std::string time1 = Onvif::getISO8601DateAndTime(0);
    Onvif mobotix("10.15.100.200", "admin", "password");
    // for (int i = 0; i < 10; i++)
    // {
        std::cout << time1 << std::endl;
        std::cout << mobotix.GetSystemDateAndTime() << std::endl;

    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }


    return 0;
}