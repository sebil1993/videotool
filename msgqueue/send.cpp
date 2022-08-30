// C Program for Message Queue (Writer Process)
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
#define MAX 10

// structure for message queue

int main(int argc, char *argv[])
{
    struct mesg_buffer
    {
        long mesg_type;
        char mesg_text[100];
    };

    mesg_buffer message = {1};
    strcpy(message.mesg_text, argv[1]);

    int msgid;

    msgid = msgget(ftok("ftokfile", 65), 0666 | IPC_CREAT);
    message.mesg_type = 1;

    msgsnd(msgid, &message, sizeof(message), 0);

    printf("Data send is : %s \n", message.mesg_text);

    return 0;
}
