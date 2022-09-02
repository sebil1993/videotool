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
    while (true)
    {
        if (nameOfFolder.find(" ") == std::string::npos)
            break;
        nameOfFolder.replace(nameOfFolder.find(" "), sizeof(" ") - 1, "_");
    }
    boost::filesystem::path path = boost::filesystem::current_path();
    path += "/storage/cameras/";
    path += nameOfFolder;

    return path;
}
std::string createSystemCallCommandForEvent(std::vector<std::string> camera)
{
    std::string event_id = "2";
    std::string systemCallCommand = "ffmpeg -hide_banner -loglevel panic -t 00:00:10 -i '$STREAMURI$' ";
    systemCallCommand += "-g 20 -b:v 2M -maxrate 2M -bufsize 1M '$OUTPUT$.mp4'";

    std::string outputFilename = "$PATH$/$FILENAME$_";
    outputFilename += "event_id";
    outputFilename += "_";
    outputFilename += event_id;
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
    // systemCallCommand.replace(systemCallCommand.find("$BASEURL$"), sizeof("$BASEURL$") - 1, path.c_str());
    systemCallCommand.replace(systemCallCommand.find("$OUTPUT$"), sizeof("$OUTPUT$") - 1, outputFilename.c_str());

    return systemCallCommand;
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

    while (strcmp(message.mesg_text, "0") != 0)
    {
        msgrcv(msgid, &message, sizeof(message), 1, 0);
        parsedEventMessage = parseEventMessage(message.mesg_text);
        camera = db.searchEntry("cameras", "*", "id", parsedEventMessage[1]);
        if (camera.size() > 0)
        {
            std::cout << createConcateSystemCallCommand(camera) << std::endl;

            system(createConcateSystemCallCommand(camera).c_str());
        }
    }
    
    std::vector<std::string> camera = db.searchEntry("cameras", "*", "id", "1");
    system(createSystemCallCommandForEvent(db.searchEntry("cameras", "*", "id", "1")).c_str());
    std::cout << "aufnahme abgeschlossen" << std::endl;
    return 0;
}
