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

std::vector<std::string> parseEventMessage(std::string msgQueue)
{
    size_t start;
    size_t end = 0;
    std::vector<std::string> parsedMessage;

    // if (msgQueue.find("EVENT_") == std::string::npos)
    // {
    //     std::cout << "hallo3" << std::endl;
    // }
    // else
    // {
    if (msgQueue.find("EVENT_") != std::string::npos)
    {
        while ((start = msgQueue.find_first_not_of("_", end)) != std::string::npos)
        {
            end = msgQueue.find("_", start);
            parsedMessage.push_back(msgQueue.substr(start, end - start));
        }
    }
    return parsedMessage;
}
boost::filesystem::path checkOrCreateDirectory(std::vector<std::string> camera)
{
    std::string nameOfCamera = "";
    nameOfCamera += camera[CAM_MANUFACTURER];
    nameOfCamera += '_';
    nameOfCamera += camera[CAM_MODEL];
    nameOfCamera += '_';
    nameOfCamera += camera[CAM_SERIALNUMBER];
    while (true)
    {
        if (nameOfCamera.find(" ") == std::string::npos)
            break;
        nameOfCamera.replace(nameOfCamera.find(" "), sizeof(" ") - 1, "_");
    }
    boost::filesystem::path filePath = boost::filesystem::current_path();
    filePath += "/storage/cameras/";
    filePath += nameOfCamera;

    return filePath;
}
std::vector<std::string> createSystemCallCommandForEvent(std::vector<std::string> camera, std::string event_id)
{
    std::vector<std::string> response;
    std::string systemCallCommand = "ffmpeg -hide_banner -loglevel panic -t 00:00:10 -i '$STREAMURI$' ";
    systemCallCommand += "-g 20 -b:v 2M '$OUTPUT$.ts'";

    std::string outputFilename = "$PATH$/$FILENAME$_event_";
    outputFilename += "event_id";
    outputFilename += "_";
    outputFilename += event_id;
    boost::filesystem::path path = checkOrCreateDirectory(camera);
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
    systemCallCommand.replace(systemCallCommand.find("$OUTPUT$"), sizeof("$OUTPUT$") - 1, outputFilename.c_str());

    response.push_back(systemCallCommand);
    response.push_back(outputFilename);
    return response;
}

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[100];
} message;

int main(int argc, char *argv[])
{

    boost::filesystem::path databasePath = boost::filesystem::current_path();
    databasePath += "/storage/database/database.db";
    DBLite db(databasePath.string());

    boost::filesystem::path ftokfilePath = boost::filesystem::current_path();
    ftokfilePath += "/msgqueue/event_MSGQ";

    int msgid = msgget(ftok(ftokfilePath.c_str(), 65), 0666 | IPC_CREAT);
    std::cout << msgid << std::endl;

    message.mesg_type = 1;

    std::vector<std::string> parsedEventMessage, camera;
    boost::filesystem::path camerapath;
    while (true)
    {
        std::cout << "[startEventRecord] waiting for event: " << std::endl;
        msgrcv(msgid, &message, sizeof(message), 1, 0);
        parsedEventMessage = parseEventMessage(message.mesg_text);

        if (strcmp(parsedEventMessage[0].c_str(), "EVENT") == 0)
        {
            std::vector<std::string> systemCallAndPath;
            camera = db.searchEntry("cameras", "*", "id", parsedEventMessage[1]);

            if (camera.size() > 0)
            {
                std::cout << "[startEventRecord] start recording for "
                          << camera[CAM_IPADDRESS]
                          << " with EVENT_ID " << parsedEventMessage[2] << std::endl;
                systemCallAndPath =createSystemCallCommandForEvent(camera, parsedEventMessage[2]); 
                system(systemCallAndPath[0].c_str());
                std::cout << "[startEventRecord] finished recording for "
                          << camera[CAM_IPADDRESS]
                          << " with EVENT_ID " << parsedEventMessage[2] << std::endl;
                db.updatePathToEvent(stoi(parsedEventMessage[2]),systemCallAndPath[1]);
            }
        }
    }

    return 0;
}
