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
    key_t key;
    int msgid;

    // ftok to generate unique key
    key = ftok("ftokfile", 65);

    // msgget creates a message queue
    // and returns identifier
    msgid = msgget(key, 0666 | IPC_CREAT);

    while (true)
    { // msgrcv to receive message
        msgrcv(msgid, &message, sizeof(message), 1, 0);

        // display the message
        // printf("Data Received is : %s \n",
        //        message.mesg_text);
        std::cout << message.mesg_text << std::endl;
        if (strcmp(message.mesg_text, "exit") == 0)
            break;
    }
    // to destroy the message queue
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
