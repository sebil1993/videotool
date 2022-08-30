#include "Onvif/Onvif.h"
#include <stdio.h>
// #include <time.h>
#include <iostream>
// #include <chrono>
#include <curl/curl.h>
// #include <ctime>
#include "DBLite.h"

// #include <cstdlib>
// #include <fstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/path.hpp>

// ich muss mir überlegen ob ich auch hier eine abfrage mache
// oder ob ich davon ausgehe, dass die kamera initialisiert wurde und die daten vorliegen
// dann könnte der prozess gestartet werden, ansonsten hab ich nochmal das selbe wie in initcam
// DATEN LIEGEN VOR -> starte prozess
// erstelle ich ordner oder wird das von init camera gemacht?
// muss hier trotzdem auf datenbank zugreifen

// das hier gibts als input wenn mans ausführt
//  ID  ipaddress    user pwd     manu  model     serialnumber  streamuri
//  15, 10.15.2.201, seb, sebseb, AXIS, M2025-LE, ACCC8EA80800, rtsp://10.15.2.201/onvif-media/media.amp?profile=profile_1_h264&sessiontimeout=60&streamtype=unicast, ,



// 1 prozess schreibt in die datenbank während der ander edie kamera am laufen hat
// dieser muss auch nachschauen

// files
// sockets
// datenbank
// msg queue

DBLite checkForDB(std::string pathToDatabase)
{
    if (!boost::filesystem::exists(pathToDatabase.c_str()))
    {
        // init db und erstelle eventtypes
        DBLite sqlDB(pathToDatabase.c_str());
        std::cout << "creating database at: " << std::endl;
        sqlDB.createTable();
        sqlDB.insertEventtypes("Zutritt");

        return sqlDB;
    }
    else
    {
        std::cout << "database exists" << std::endl;
        DBLite sqlDB(pathToDatabase.c_str());

        return sqlDB;
    }
}

boost::filesystem::path checkOrCreateDirectory(std::vector<std::string> camera)
{
    std::string nameOfFolder = "";
    nameOfFolder += camera[CAM_MANUFACTURER];
    nameOfFolder += '_';
    nameOfFolder += camera[CAM_MODEL];
    nameOfFolder += '_';
    nameOfFolder += camera[CAM_SERIALNUMBER];

    boost::filesystem::path path = boost::filesystem::current_path();
    path += "/storage/cameras/";
    path += nameOfFolder;
    if (!boost::filesystem::exists(path))
    {
        boost::filesystem::create_directory(path);
    }

    return path;
}

std::string createSystemCallCommand(){
    std::string systemCallCommand;
    //"ffmpeg -t 00:00:15 -i '$CAMERASTREAM$'
    // -c:v libx264 -flags +cgop -g 24 
    //-hls_flags delete_segments -hls_time 4 -hls_list_size 4 
    //$PLAYLIST$.m3u8";

    //systemcall soll aus mehreren parametern bestehen
    //ffmpeg
    //dauer der aufnahme (wird aber rausgeschmissen)
    //inputstream wird der rtsp link sein, den man über die kamera erhält
    //
    //codec und optionen
    //hls ausgabeformat oder eventuell segmente
    //name der ausgabe
    systemCallCommand = "ffmpeg ";


    return systemCallCommand;
}
int main(int argc, char *argv[])
{
    DBLite db = checkForDB("storage/database/database.db");
    const char *camera_id;
    if (argc > 1)
    {
        camera_id = argv[1];
    }
    else{
        std::cout << "no viable input given" << std::endl;
        exit(0);
    }
    std::vector<std::string> camera = db.searchEntry("cameras", "*", "ID", camera_id);
    for(std::string str: camera){
        std::cout << str << std::endl;
    }
    // 16
    // 10.15.100.200
    // admin
    // password
    // MOBOTIX MOVE
    // MOVE-VD1A-2-IR
    // 0003c5c025b4
    // rtsp://10.15.100.200/h264
    //
    checkOrCreateDirectory(camera);
    std::string playlistName = "output";
    std::string systemCallCommand = "ffmpeg -t 00:00:15 -i '$CAMERASTREAM$' -c:v libx264 -flags +cgop -g 24 -hls_flags delete_segments -hls_time 4 -hls_list_size 4 $PLAYLIST$.m3u8";
    systemCallCommand.replace(systemCallCommand.find("$CAMERASTREAM$"), sizeof("$CAMERASTREAM$") - 1, camera[CAM_STREAMURI].c_str());
    systemCallCommand.replace(systemCallCommand.find("$PLAYLIST$"), sizeof("$PLAYLIST$") - 1, playlistName.c_str());
    system(systemCallCommand.c_str());

    systemCallCommand = "cat `cat $PLAYLIST$.m3u8 | grep .ts` | > concated_$PLAYLIST$.ts";
    systemCallCommand.replace(systemCallCommand.find("$PLAYLIST$"), sizeof("$PLAYLIST$") - 1, playlistName.c_str());
    system(systemCallCommand.c_str());

    return 0;
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

// (̅_̅_̅((̅_̅_ ̅_̅_̅_̅_̅()
// Onvif mobotix("10.15.100.200", "admin", "password"); // serverCAM1
// Onvif mobotix("10.15.100.201", "admin", "password"); // serverCAM2
// Onvif axis("10.15.2.200", "kentix", "kentix");       // strassenCAM
// Onvif axis("10.15.2.201", "seb", "sebseb");          // parkplatzCAM
// Onvif axis("10.15.3.201", "", "");                   // webinarCAM
// Onvif axis("10.15.2.201", "seb", "sebseb");
// axis.init(false, false);
// axis.getAllInfos();

// path += "/storage/cameras/";
// path += axis.getUniqueDeviceName();
// std::cout << path << std::endl;
// std::cout << boost::filesystem::exists(path) << std::endl;
// boost::filesystem::create_directories(path);
// return 0;