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

// ich muss mir überlegen ob ich auch hier eine abfrage mache
// oder ob ich davon ausgehe, dass die kamera initialisiert wurde und die daten vorliegen
// dann könnte der prozess gestartet werden, ansonsten hab ich nochmal das selbe wie in initcam
// DATEN LIEGEN VOR -> starte prozess
// erstelle ich ordner oder wird das von init camera gemacht?
// muss hier trotzdem auf datenbank zugreifen

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
        std::cout << "creating database at: " << pathToDatabase << std::endl;
        sqlDB.createTable();
        sqlDB.insertEventTypes("Zutritt");

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

std::string createSystemCallCommandForBuffer(std::vector<std::string> camera, std::string timestamp)
{
    std::string systemCallCommand = "ffmpeg -i '$STREAMURI$' ";
    // systemCallCommand += "-g 20 -r 4 -b:v 2M -maxrate 2M -bufsize 1M -f hls -hls_flags delete_segments -hls_list_size 12 '$OUTPUT$.m3u8'";
    systemCallCommand += "-g 20 -b:v 2M -maxrate 2M -bufsize 1M -f hls -hls_flags delete_segments -hls_time 5 -hls_list_size 12 '$OUTPUT$.m3u8'";

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
    outputFilename.replace(outputFilename.find("$FILENAME$"), sizeof("$FILENAME$") - 1, timestamp.c_str());
    cameraStreamURI.replace(cameraStreamURI.find("//"), sizeof("//") - 1, credentials.c_str());
    systemCallCommand.replace(systemCallCommand.find("$STREAMURI$"), sizeof("$STREAMURI$") - 1, escapeAmpersand(cameraStreamURI).c_str());
    systemCallCommand.replace(systemCallCommand.find("$OUTPUT$"), sizeof("$OUTPUT$") - 1, outputFilename.c_str());

    return systemCallCommand;
}
std::string createSystemCallCommandForEvent(std::vector<std::string> camera, std::vector<std::string> event)
{
    std::string systemCallCommand = "ffmpeg -i '$STREAMURI$' ";
    systemCallCommand += "-g 20 -r 4 -b:v 2M -maxrate 2M -bufsize 1M -f hls -hls_flags delete_segments -hls_list_size 12 '$OUTPUT$.m3u8'";

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
    outputFilename.replace(outputFilename.find("$FILENAME$"), sizeof("$FILENAME$") - 1, event[1].c_str());
    cameraStreamURI.replace(cameraStreamURI.find("//"), sizeof("//") - 1, credentials.c_str());
    systemCallCommand.replace(systemCallCommand.find("$STREAMURI$"), sizeof("$STREAMURI$") - 1, escapeAmpersand(cameraStreamURI).c_str());
    systemCallCommand.replace(systemCallCommand.find("$OUTPUT$"), sizeof("$OUTPUT$") - 1, outputFilename.c_str());

    return systemCallCommand;
}

std::vector<std::string> parseEventMessage(std::string msgQueue)
{
    size_t start;
    size_t end = 0;
    std::vector<std::string> parsedMessage;
    if (msgQueue.find("event_") != std::string::npos)
    {
        while ((start = msgQueue.find_first_not_of("_", end)) != std::string::npos)
        {
            end = msgQueue.find("_", start);
            parsedMessage.push_back(msgQueue.substr(start, end - start));
        }
    }
    return parsedMessage;
}

// zunächst wird die kamera initialisiert, dabei wird in der datenbank nachgeschaut ob die IP vorliegt
// weiterhin wird ein ordner für die kamera erstellt die aus hersteller, modell und seriennummer besteht
// dann wird die aufnahme gestartet und als playlist+segmente im ordner abgelegt

// passiert ein event
// wird alles bisherige aus der playlist mit cat konkateniert
// ein zweiter prozess wird gestartet für 60s
// es liegen dann 2 dateien vor, playlist_concat und event_timestamp
// diese werden nochmal verbunden

// bisher werden die dateien nicht in die db geschrieben, da angenommen wird dass PHP den ordner selbst herleiten kann
// überlegung:  beide dateien einzeln in db schreiben
//              -> php müsste nicht die ordner herleiten, sondern wüsste auf jeden fall wo es nachzuschauen hat
//              -> so müsste man sie nicht verbinden und könnte verschiedene framerates beibehalten

struct mesg_buffer
{
    long mesg_type;
    char mesg_text[100];
} message;

int main(int argc, char *argv[])
{
    DBLite db = checkForDB("storage/database/database.db");
    
    int msgid = msgget(ftok("/Users/kentix/Documents/git/videotool/msgqueue/ftokfile", 65), 0666 | IPC_CREAT);
    std::vector<std::string> parsedEventMessage;

    message.mesg_type = 1;
    std::cout << "waiting for event: " << std::endl;
    while (strcmp(message.mesg_text, "exit") != 0)
    {
        msgrcv(msgid, &message, sizeof(message), 1, 0);
        parsedEventMessage = parseEventMessage(message.mesg_text);
        if (parsedEventMessage.size() > 0)
        {
            std::cout << "eventtype: " << parsedEventMessage[1] << " camera_id: " << parsedEventMessage[2] << std::endl;
            break;
        }
    }

    msgctl(msgid, IPC_RMID, NULL);
    int event_id = db.insertEvent(stoi(parsedEventMessage[1]), stoi(parsedEventMessage[2]));
    // db.showTable("events");

    boost::filesystem::path path = "storage/cameras/beispielkamera";
    db.updatePathToEvent(event_id, path.string());
    // db.showTable("events");
    // std::vector<std::string> event = db.searchEntry("events", "*", "", "");

    // if (argc > 1)
    // {
    //     camera_id = argv[1];
    // }
    // else
    // {
    //     std::cout << "no viable input given" << std::endl;
    //     exit(0);
    // }
    // std::vector<std::string> camera = db.searchEntry("cameras", "*", "ID", camera_id);

    // std::string systemCallCommand = createSystemCallCommandForBuffer(camera, "EVENT_ZAHL");

    // std::cout << systemCallCommand << std::endl;
    // ID: 16 benutzen

    return 0;
}