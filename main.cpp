#include "Onvif/Onvif.h"
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <chrono>
#include <curl/curl.h>
#include <ctime>

// #include <thread>
// #include <deque>

#include <cstdlib>
#include <fstream>

// #include <boost/filesystem/operations.hpp>
// #include <boost/filesystem/directory.hpp>
// #include <boost/filesystem/path.hpp>

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        std::string ipAddress = argv[2];
    }
    // if (!checkCamera(CameraIP))
    // {
    //     "Fehler: konnte nichts in DB finden";
    //     "Bitte %username und %password angeben";
    //     if (!initCamera(username, password))
    //     {
    //         "Fehler: konnte nicht initialisieren";
    //         exit();
    //     }
    // }

    // process(camera.streamlink){
    //     starteFFMPEG(camera.streamlink){
    //         while(buffering){
    //             nehmeDieVideosAufInBuffer();
    //             wennBufferVollVerwerfe();
    //             if(Signal){
    //                 system(cat(dateien_playlist))
    //                 starteFFMPEG(camera.streamlink, 1min)
    //             }
    //         }
    //     }
    // }
}
// (̅_̅_̅((̅_̅_ ̅_̅_̅_̅_̅()
// Onvif mobotix("10.15.100.200", "admin", "password"); // serverCAM1
// Onvif mobotix("10.15.100.201", "admin", "password"); // serverCAM2
// Onvif axis("10.15.2.200", "kentix", "kentix");       // strassenCAM
// Onvif axis("10.15.2.201", "seb", "sebseb");          // parkplatzCAM
// Onvif axis("10.15.3.201", "", "");                   // webinarCAM
// Onvif axis("10.15.2.201", "seb", "sebseb");
// axis.init(false, false);
// axis.getAllInfos();

// auto path = boost::filesystem::current_path();
// path += "/storage/cameras/";
// path += axis.getUniqueDeviceName();
// std::cout << path << std::endl;
// std::cout << boost::filesystem::exists(path) << std::endl;
// boost::filesystem::create_directories(path);
// return 0;