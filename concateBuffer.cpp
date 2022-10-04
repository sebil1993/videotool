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
    boost::filesystem::path cameraPath = boost::filesystem::current_path().parent_path();

    cameraPath += "/storage/app/cameras/";
    cameraPath += nameOfCamera;

    return cameraPath;
}
std::string getFilenameForBuffer(std::vector<std::string> camera, int event_id)
{
    std::string filename;
    filename = camera[CAM_SERIALNUMBER];
    filename += "_event_id_";
    filename += std::to_string(event_id);
    filename += "_buffer.ts";
    return filename;
}
std::string getFilenameForEvent(std::vector<std::string> camera, int event_id)
{
    std::string filename;
    filename = camera[CAM_SERIALNUMBER];
    filename += "_event_id_";
    filename += std::to_string(event_id);
    filename += "_event.ts";
    return filename;
}
std::string createSystemCallCommandForConcate(std::vector<std::string> camera, int event_id, std::string namePrefix)
{
    boost::filesystem::path cameraPath = getDirectory(camera);
    std::cout << "[concateBuffer] concating " << namePrefix << " for "
              << camera[CAM_IPADDRESS]
              << " with EVENT_ID " << event_id << std::endl;

    cameraPath += "/";
    boost::filesystem::path filePathBuffer = cameraPath;
    filePathBuffer += camera[CAM_SERIALNUMBER];
    filePathBuffer += ".m3u8";

    std::string concateCall = "cat `cat '$FILEPATHBUFFER$' | grep .ts` | ffmpeg -hide_banner -loglevel panic -fflags +discardcorrupt -f mpegts -i pipe: -c:v libx264 -b:v 500K '$CAMERAPATH$$SERIALNUMBER$_event_id_$EID$_$PREFIX$.ts'";
    concateCall.replace(concateCall.find("$FILEPATHBUFFER$"), sizeof("$FILEPATHBUFFER$") - 1, filePathBuffer.c_str());
    concateCall.replace(concateCall.find("$CAMERAPATH$"), sizeof("$CAMERAPATH$") - 1, cameraPath.c_str());
    concateCall.replace(concateCall.find("$SERIALNUMBER$"), sizeof("$SERIALNUMBER$") - 1, camera[CAM_SERIALNUMBER].c_str());
    concateCall.replace(concateCall.find("$EID$"), sizeof("$EID$") - 1, std::to_string(event_id).c_str());
    concateCall.replace(concateCall.find("$PREFIX$"), sizeof("$PREFIX$") - 1, namePrefix.c_str());

    return concateCall;
}

void concateBufferAndEvent(std::vector<std::string> camera, int event_id)
{

    boost::filesystem::path databasePath;
    databasePath = boost::filesystem::absolute(databasePath);
    databasePath = databasePath.parent_path();
    databasePath += "/database/database.sqlite";
    DBLite db(databasePath.string());

    boost::filesystem::path cameraPath;
    cameraPath = getDirectory(camera);
    boost::filesystem::path buffer_event = cameraPath;
    boost::filesystem::path event_event = cameraPath;
    buffer_event += "/";
    buffer_event += getFilenameForBuffer(camera, event_id);
    event_event += "/";
    event_event += getFilenameForEvent(camera, event_id);
    cameraPath += "/events";

    if (!boost::filesystem::exists(cameraPath))
    {
        boost::filesystem::create_directories(cameraPath);
    }

    std::string syscall = "cat $BUFFEREVENTID$ $EVENTEVENTID$ | ffmpeg -hide_banner -loglevel panic -fflags +discardcorrupt -f mpegts -i pipe: -c:v libx264 -b:v 500K '$CAMERAEVENTSPATH$/event_id_$EVENTID$.mp4'";
    syscall.replace(syscall.find("$BUFFEREVENTID$"), sizeof("$BUFFEREVENTID$") - 1, buffer_event.c_str());
    syscall.replace(syscall.find("$EVENTEVENTID$"), sizeof("$EVENTEVENTID$") - 1, event_event.c_str());
    syscall.replace(syscall.find("$CAMERAEVENTSPATH$"), sizeof("$CAMERAEVENTSPATH$") - 1, cameraPath.c_str());
    syscall.replace(syscall.find("$EVENTID$"), sizeof("$EVENTID$") - 1, std::to_string(event_id).c_str());

    std::string filepath = "$CAMERAEVENTSPATH$/event_id_$EVENTID$.mp4";
    filepath.replace(filepath.find("$CAMERAEVENTSPATH$"), sizeof("$CAMERAEVENTSPATH$") - 1, cameraPath.c_str());
    filepath.replace(filepath.find("$EVENTID$"), sizeof("$EVENTID$") - 1, std::to_string(event_id).c_str());

    // std::cout << syscall.c_str() << std::endl;
    std::cout << "[concateBuffer] concating buffer and event " << std::endl;

    system(syscall.c_str());
    std::cout << "[concateBuffer] finished concating buffer and event " << std::endl;
    boost::filesystem::remove(buffer_event.c_str());
    boost::filesystem::remove(event_event.c_str());
    db.updatePathToEvent(event_id, filepath);
    // db.updatePathToEvent(event_id, filepath);
}

void systemCallWithDelay(std::vector<std::string> camera, int event_id, long long waitTime)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20000 - waitTime));
    system(createSystemCallCommandForConcate(camera, event_id, "event").c_str());
    // std::cout << createSystemCallCommandForConcate(camera, event_id, "event") << std::endl;
    std::cout << "[concateBuffer] finished event recording" << std::endl;
    concateBufferAndEvent(camera, event_id);
    std::cout << "[concateBuffer] waiting for event: " << std::endl;
}

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[100];
} message;

int main(int argc, char *argv[])
{

    boost::filesystem::path databasePath;
    databasePath = boost::filesystem::absolute(databasePath);
    databasePath = databasePath.parent_path();
    databasePath += "/database/database.sqlite";
    DBLite db(databasePath.string());

    // std::cout << databasePath << std::endl;

    boost::filesystem::path ftokfilePath;
    ftokfilePath = boost::filesystem::absolute(ftokfilePath);
    // ftokfilePath = "/Users/kentix/Documents/git/videotool-webinterface/app/bin/concate_MSGQ";
    ftokfilePath = ftokfilePath.parent_path();
    ftokfilePath += "/app/bin/concate_MSGQ";
    // std::cout << ftokfilePath << std::endl;
    key_t key = ftok(ftokfilePath.c_str(), 'A');

    int msgid = msgget(key, 0666 | IPC_CREAT);
    // std::cout << "ftok: " << key << std::endl
    //           << "msgid: " << msgid << std::endl;

    std::vector<std::string> parsedConcateMessage, camera;
    boost::filesystem::path cameraPath;

    while (true)
    {
        std::cout << "[concateBuffer] waiting for event: " << std::endl;
        msgrcv(msgid, &message, sizeof(message.mesg_text) - 1, 0, 0);
        parsedConcateMessage = parseConcateMessage(message.mesg_text);

        if (strcmp(parsedConcateMessage[0].c_str(), "CONCATE") == 0)
        {

            camera = db.searchEntry("cameras", "*", "id", parsedConcateMessage[1]);

            if (camera.size() > 0)
            {

                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                system(createSystemCallCommandForConcate(camera, stoi(parsedConcateMessage[2]), "buffer").c_str());
                std::cout << "[concateBuffer] finished buffer recording" << std::endl;

                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

                long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
                std::thread syscallWithDelay(systemCallWithDelay, camera, stoi(parsedConcateMessage[2]), timeToFinish);
                syscallWithDelay.detach();
            }
        }
    }
}
