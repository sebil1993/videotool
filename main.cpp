#include "Onvif/Onvif.h"
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <ctime>

#include <deque>
#include <cstdlib>
#include <fstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/path.hpp>

int main()
{
    // (̅_̅_̅((̅_̅_ ̅_̅_̅_̅_̅()
    // Onvif mobotix("10.15.100.200", "admin", "password"); // serverCAM1
    // Onvif mobotix("10.15.100.201", "admin", "password"); // serverCAM2
    // Onvif axis("10.15.2.200", "kentix", "kentix");       // strassenCAM
    // Onvif axis("10.15.2.201", "seb", "sebseb");          // parkplatzCAM
    // Onvif axis("10.15.3.201", "", "");                   // webinarCAM

    Onvif axis("10.15.2.201", "seb", "sebseb");
    axis.init(false, false);
    axis.getAllInfos();

    auto path = boost::filesystem::current_path();
    path += "/storage/cameras/";
    path += axis.getUniqueDeviceName();
    std::cout << path << std::endl;
    std::cout << boost::filesystem::exists(path) << std::endl;
    boost::filesystem::create_directories(path);
    return 0;
}