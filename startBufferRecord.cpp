#include "Onvif/Onvif.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
#include <curl/curl.h>
#include "DBLite.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/path.hpp>

std::string escapeAmpersand(std::string &streamURI)
{
    while (true)
    {
        if (streamURI.find("&") == std::string::npos)
            break;
        streamURI.replace(streamURI.find("&"), sizeof("&") - 1, "%26");
    }

    return streamURI;
}

boost::filesystem::path checkOrCreateDirectory(std::vector<std::string> camera)
{
    std::string nameOfFolder = "";
    nameOfFolder += camera[CAM_MANUFACTURER];
    nameOfFolder += '_';
    nameOfFolder += camera[CAM_MODEL];
    nameOfFolder += '_';
    nameOfFolder += camera[CAM_SERIALNUMBER];
    while (true)
    {
        if (nameOfFolder.find(" ") == std::string::npos)
            break;
        nameOfFolder.replace(nameOfFolder.find(" "), sizeof(" ") - 1, "_");
    }

    boost::filesystem::path path = boost::filesystem::absolute("startBufferRecord");
    path = path.parent_path().parent_path();

    path += "/storage/app/cameras/";
    path += nameOfFolder;
    if (!boost::filesystem::exists(path))
    {
    std::cout << path << std::endl;
        boost::filesystem::create_directory(path);
    }

    return path;
}

std::string createSystemCallCommandForBuffer(std::vector<std::string> camera)
{
    std::string systemCallCommand = "ffmpeg -hide_banner -loglevel error -i '$STREAMURI$' ";
    // std::string systemCallCommand = "ffmpeg -i '$STREAMURI$' ";
    systemCallCommand += "-g 20 -b:v 2M -maxrate 2M -bufsize 1M -f hls -hls_flags delete_segments -hls_base_url '$BASEURL$/' -hls_time 2 -hls_list_size 10 '$OUTPUT$.m3u8'";

    std::string outputFilename = "$PATH$/$FILENAME$";
    auto path = checkOrCreateDirectory(camera);
    std::string cameraStreamURI = camera[CAM_STREAMURI];
    std::string credentials;
    credentials = "//";
    credentials += camera[CAM_USERNAME];
    credentials += ":";
    credentials += camera[CAM_PASSWORD];
    credentials += "@";

    outputFilename.replace(outputFilename.find("$PATH$"), sizeof("$PATH$") - 1, path.c_str());
    outputFilename.replace(outputFilename.find("$FILENAME$"), sizeof("$FILENAME$") - 1, camera[CAM_SERIALNUMBER].c_str());
    cameraStreamURI.replace(cameraStreamURI.find("//"), sizeof("//") - 1, credentials.c_str());

    systemCallCommand.replace(systemCallCommand.find("$STREAMURI$"), sizeof("$STREAMURI$") - 1, escapeAmpersand(cameraStreamURI).c_str());
    systemCallCommand.replace(systemCallCommand.find("$BASEURL$"), sizeof("$BASEURL$") - 1, path.c_str());
    systemCallCommand.replace(systemCallCommand.find("$OUTPUT$"), sizeof("$OUTPUT$") - 1, outputFilename.c_str());

    return systemCallCommand;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "[startBufferRecord] no camera given" << std::endl;
        exit(0);
    }

    boost::filesystem::path databasePath;// = argv[0];
    databasePath = boost::filesystem::absolute(databasePath);
    // std::cout << databasePath << std::endl;
    databasePath = databasePath.parent_path();
    databasePath += "/database/database.sqlite";
    DBLite db(databasePath.string());

        // std::cout << "danach" << std::endl;
    std::vector<std::string> camera = db.searchEntry("cameras", "*", "ip_address", argv[1]);
    if (camera.size() == 0)
    {
        // db.showTable("cameras");
        std::cout << "[startBufferRecord] no camera with IP: " << argv[1] << " found!" << std::endl;
        camera = db.searchEntry("cameras", "*", "id", argv[1]);
        if (camera.size() == 0)
        {
            std::cout << "[startBufferRecord] no camera with ID: " << argv[1] << " found!" << std::endl;
            exit(0);
        }
    }
    std::cout << createSystemCallCommandForBuffer(camera) << std::endl;
    std::cout << "[startBufferRecord] starting buffer for: " << camera[CAM_IPADDRESS] << std::endl;
    // std::cout << createSystemCallCommandForBuffer(camera).c_str() << std::endl;
    system(createSystemCallCommandForBuffer(camera).c_str());

    return 0;
}