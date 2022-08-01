#include <ctime>
#include <iostream>
#include <chrono>
#include <time.h>
#include <thread>
#include <chrono>

time_t ISO8601toTimestamp(std::string timestamp)
{
    struct tm timeCam;
    strptime(timestamp.c_str(), "%Y-%m-%d %H:%M:%S", &timeCam);
    time_t t = mktime(&timeCam);

    return t;
}

std::string getISO8601DateAndTime()
{
    time_t now;
    struct tm ts;
    char buf[28];

    time(&now);
    // now -= deltaTime;
    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.000Z", &ts);
    std::cout << mktime(&ts) << std::endl; //=> GIBT ZEIT ALAS ZEITSTEMPEL ZÜRÜCK
    std::string ISO8601(buf);
    // std::cout << ISO8601 << std::endl;
    return ISO8601;
}

/*
    std::string timeCamera = "2022-07-28 10:34:31.000Z";
    std::string timeUser = "2022-07-28 12:34:31.000Z";

    std::cout << timeCamera << " -> " << ISO8601toTimestamp(timeCamera)  << std::endl;
    std::cout << timeUser << " -> " << ISO8601toTimestamp(timeUser)  << std::endl;

    std::cout << "difference is:" << ISO8601toTimestamp(timeCamera) - ISO8601toTimestamp(timeUser) << std::endl;

*/
using namespace std;

void signalHandler(int signum)
{
    cout << "Interrupt signal (" << signum << ") received.\n";

    // cleanup and close up stuff here
    // terminate program

    exit(signum);
}

int main()
{
    // register signal SIGINT and signal handler
    signal(SIGINT, signalHandler);

    while (1)
    {
        cout << "Going to sleep...." << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}

// int main ()
// {
//   time_t now;
//   struct tm newyear;
//   double seconds;

//   time(&now);  /* get current time; same as: now = time(NULL)  */

//   newyear = *localtime(&now);

//   newyear.tm_hour = 0; newyear.tm_min = 0; newyear.tm_sec = 0;
//   newyear.tm_mon = 0;  newyear.tm_mday = 1;

//   seconds = difftime(now,mktime(&newyear));

//   printf ("%.f seconds since new year in the current timezone.\n", seconds);

//   return 0;
// }