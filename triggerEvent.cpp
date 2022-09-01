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

int main(int argc, char *argv[])
{
    struct mesg_buffer
    {
        long mesg_type;
        char mesg_text[100];
    };

    mesg_buffer message;
    message.mesg_type = 1;

    boost::filesystem::path ftokfilePath = boost::filesystem::current_path();
    ftokfilePath += "/msgqueue/ftokfile";
    int msgid = msgget(ftok(ftokfilePath.c_str(), 65), 0666 | IPC_CREAT);
    std::cout << msgid << std::endl;

    int camera_id = 2;
    int event_id = 2;
    std::string messageString = "EVENT_$CID$_$EID$";
    messageString.replace(messageString.find("$CID$"), sizeof("$CID$") - 1, std::to_string(camera_id));
    messageString.replace(messageString.find("$EID$"), sizeof("$EID$") - 1, std::to_string(event_id));
    std::cout << messageString << std::endl;

    strcpy(message.mesg_text, messageString.c_str());
    
    msgsnd(msgid, &message, sizeof(message), 0);

    printf("Data send is : %s \n", message.mesg_text);

    return 0;
}