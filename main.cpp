#include "Onvif/Onvif.h"
#include <stdio.h>  /* printf */
#include <time.h>   /* time_t, struct tm, difftime, time, mktime */
#include <iostream> // std::cout, std::endl
#include <thread>   // std::this_thread::sleep_for
#include <chrono>   // std::chrono::seconds
#include <curl/curl.h>
#include <ctime>

#include <deque>
#include <cstdlib>
#include <fstream>

int main()
{
    // Onvif mobotix("10.15.100.200", "admin", "password"); // serverCAM1
    // Onvif mobotix("10.15.100.201", "admin", "password"); // serverCAM2
    // Onvif axis("10.15.2.200", "kentix", "kentix");       // strassenCAM
    // Onvif axis("10.15.2.201", "seb", "sebseb");          // parkplatzCAM
    // Onvif axis("10.15.3.201", "", "");                   // webinarCAM

    Onvif axis("10.15.2.201", "seb", "sebseb");
    axis.init(false,false);
    axis.getAllInfos();
    return 0;
}