#include "Onvif/Onvif.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
#include <curl/curl.h>
#include "DBLite.h"
#include <thread>
#include <chrono>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/path.hpp>

std::vector<std::string> parseConcateMessage(std::string msgQueue)
{
    size_t start;
    size_t end = 0;
    std::vector<std::string> parsedMessage;

    if (msgQueue.find("CONCATE_") != std::string::npos)
    {
        while ((start = msgQueue.find_first_not_of("_", end)) != std::string::npos)
        {
            end = msgQueue.find("_", start);
            parsedMessage.push_back(msgQueue.substr(start, end - start));
        }
    }
    return parsedMessage;
}
boost::filesystem::path getDirectory(std::vector<std::string> camera)
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
    boost::filesystem::path cameraPath = boost::filesystem::current_path();
    cameraPath += "/storage/cameras/";
    cameraPath += nameOfCamera;

    return cameraPath;
}
std::string createSystemCallCommandForConcate(std::vector<std::string> camera, std::string event_id)
{
    boost::filesystem::path cameraPath = getDirectory(camera);
    cameraPath += "/";
    boost::filesystem::path filePathBuffer = cameraPath;
    filePathBuffer += camera[CAM_SERIALNUMBER];
    filePathBuffer += ".m3u8";

    std::string concateCall = "cat `cat '$FILEPATHBUFFER$' | grep .ts` > '$CAMERAPATH$$SERIALNUMBER$_buffer_event_id_$EID$.ts'";

    concateCall.replace(concateCall.find("$FILEPATHBUFFER$"), sizeof("$FILEPATHBUFFER$") - 1, filePathBuffer.c_str());
    concateCall.replace(concateCall.find("$CAMERAPATH$"), sizeof("$CAMERAPATH$") - 1, cameraPath.c_str());
    concateCall.replace(concateCall.find("$SERIALNUMBER$"), sizeof("$SERIALNUMBER$") - 1, camera[CAM_SERIALNUMBER].c_str());
    concateCall.replace(concateCall.find("$EID$"), sizeof("$EID$") - 1, event_id.c_str());

    return concateCall;
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
    ftokfilePath += "/msgqueue/concate_MSGQ";

    int msgid = msgget(ftok(ftokfilePath.c_str(), 65), 0666 | IPC_CREAT);
    std::cout << msgid << std::endl;

    message.mesg_type = 1;
    std::cout << "[concateBuffer] waiting for event: " << std::endl;

    std::vector<std::string> parsedConcateMessage, camera;
    boost::filesystem::path cameraPath;
    while (true)
    {
        msgrcv(msgid, &message, sizeof(message), 1, 0);
        parsedConcateMessage = parseConcateMessage(message.mesg_text);

        if (strcmp(parsedConcateMessage[0].c_str(), "CONCATE") == 0)
        {
            camera = db.searchEntry("cameras", "*", "id", parsedConcateMessage[1]);

            if (camera.size() > 0)
            {
                cameraPath = getDirectory(camera);
                std::cout << "[concateBuffer] concating buffer for "
                          << camera[CAM_IPADDRESS]
                          << " with EVENT_ID " << parsedConcateMessage[2] << std::endl;
                // std::this_thread::sleep_for(std::chrono::seconds(6));

                system(createSystemCallCommandForConcate(camera, parsedConcateMessage[2]).c_str());
                // db.updatePathToEvent(stoi(parseConcateMessage[2].c_str()))
            }
        }
    }
}