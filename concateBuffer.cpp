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

std::vector<std::string> parseEventMessage(std::string msgQueue)
{
    size_t start;
    size_t end = 0;
    std::vector<std::string> parsedMessage;
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
std::string createConcateSystemCallCommand(std::vector<std::string> camera)
{
    boost::filesystem::path camerapath = checkOrCreateDirectory(camera);
    camerapath += "/";
    boost::filesystem::path filepath = camerapath;
    filepath += camera[CAM_SERIALNUMBER];
    filepath += ".m3u8";

    std::string concateCall = "cat `cat $FILEPATH$ | grep .ts` | > '$CAMERAPATH$concated_$SERIALNUMBER$.ts'";

    concateCall.replace(concateCall.find("$FILEPATH$"), sizeof("$FILEPATH$") - 1, filepath.c_str());
    concateCall.replace(concateCall.find("$CAMERAPATH$"), sizeof("$CAMERAPATH$") - 1, camerapath.c_str());
    concateCall.replace(concateCall.find("$SERIALNUMBER$"), sizeof("$SERIALNUMBER$") - 1, camera[CAM_SERIALNUMBER].c_str());

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
    ftokfilePath += "/msgqueue/ftokfile";

    int msgid = msgget(ftok(ftokfilePath.c_str(), 65), 0666 | IPC_CREAT);
    std::cout << msgid << std::endl;

    message.mesg_type = 1;
    std::cout << "waiting for event: " << std::endl;

    std::vector<std::string> parsedEventMessage, camera;
    boost::filesystem::path camerapath;

    // while (strcmp(message.mesg_text, "exit_concate") != 0)
    // {
    msgrcv(msgid, &message, sizeof(message), 1, 0);
    parsedEventMessage = parseEventMessage(message.mesg_text);
    camera = db.searchEntry("cameras", "*", "id", parsedEventMessage[1]);
    std::cout << createConcateSystemCallCommand(camera) << std::endl;

    system(createConcateSystemCallCommand(camera).c_str());
    // }
}
