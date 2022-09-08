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
struct mesg_buffer
{
    long mesg_type;
    char mesg_text[100];
};

int main(int argc, char *argv[])
{
    boost::filesystem::path databasePath = boost::filesystem::current_path();
    databasePath += "/storage/database/database.db";
    DBLite db(databasePath.string());
    if (argc == 1 || strcmp(argv[1], "exit") == 0)
    {
        std::cout << "exiting" << std::endl;
        exit(0);
    }
    int camera_id = std::stoi(argv[1]);
    int event_id = db.insertEvent(camera_id, 1);

    mesg_buffer messageConcate;
    messageConcate.mesg_type = 1;
    // mesg_buffer messageEvent;
    // messageEvent.mesg_type = 1;

    boost::filesystem::path ftokfilePath = boost::filesystem::current_path();
    ftokfilePath += "/msgqueue/concate_MSGQ";
    int concateMsgid = msgget(ftok(ftokfilePath.c_str(), 65), 0666 | IPC_CREAT);
    std::cout << "concateMsgid " << concateMsgid << std::endl;

    std::string concateMessageString = "CONCATE_$CID$_$EID$";
    concateMessageString.replace(concateMessageString.find("$CID$"), sizeof("$CID$") - 1, std::to_string(camera_id));
    concateMessageString.replace(concateMessageString.find("$EID$"), sizeof("$EID$") - 1, std::to_string(event_id));

    strcpy(messageConcate.mesg_text, concateMessageString.c_str());
    msgsnd(concateMsgid, &messageConcate, sizeof(messageConcate), 0);
    printf("[Concate Data] send is : %s \n", messageConcate.mesg_text);
    // std::cout << concateMessageString << "  " << eventMessageString << std::endl;

    // ftokfilePath = boost::filesystem::current_path();
    // ftokfilePath += "/msgqueue/event_MSGQ";
    // int eventMsgid = msgget(ftok(ftokfilePath.c_str(), 65), 0666 | IPC_CREAT);
    // std::cout << "eventMsgid " << eventMsgid << std::endl;

    // std::string eventMessageString = "EVENT_$CID$_$EID$";
    // eventMessageString.replace(eventMessageString.find("$CID$"), sizeof("$CID$") - 1, std::to_string(camera_id));
    // eventMessageString.replace(eventMessageString.find("$EID$"), sizeof("$EID$") - 1, std::to_string(event_id));

    // strcpy(messageEvent.mesg_text, eventMessageString.c_str());
    // msgsnd(eventMsgid, &messageEvent, sizeof(messageEvent), 0);
    // printf("[Event Data] send is : %s \n", messageEvent.mesg_text);

    return 0;
}