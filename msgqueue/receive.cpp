// C Program for Message Queue (Reader Process)
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>

// structure for message queue
struct mesg_buffer
{
    long mesg_type;
    char mesg_text[100];
} message;

int main()
{
    message.mesg_type = 1;
    int msgid;

    msgid = msgget(ftok("ftokfile", 65), 0666 | IPC_CREAT);

    while (strcmp(message.mesg_text, "exit") != 0)
    { 
        msgrcv(msgid, &message, sizeof(message), 1, 0);
        std::cout << message.mesg_text << std::endl;
    }

    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
