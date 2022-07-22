#include <stdio.h>
#include <iostream>
#include <string>
#include <pthread.h>
// #include <thread>

void *run(void *tid)
{
    int id = (long) tid;
    std::cout << "test " << id << std::endl;
    pthread_exit(NULL);
}

int main()
{

    pthread_t threads[6];
    int count = 12;
    for (int i = 0; i < count; i++)
    {
        std::cout << "creating thread " << i << std::endl;

        pthread_create(&threads[i], NULL, run, (void*)i);
    }
    return 0;
}