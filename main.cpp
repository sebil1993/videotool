#include "Onvif/Onvif.h"
#include <stdio.h>  /* printf */
#include <time.h>   /* time_t, struct tm, difftime, time, mktime */
#include <iostream> // std::cout, std::endl
#include <thread>   // std::this_thread::sleep_for
#include <chrono>   // std::chrono::seconds
#include <curl/curl.h>

#include <cstdlib>

size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    FILE *writeData = (FILE *)userdata;
    if (!writeData)
    {
        std::cout << "nothing to write" << std::endl;
        return 0;
    }
    size_t written = fwrite((FILE *)ptr, size, nmemb, writeData);
    return written;
}
size_t writeImageToFile(std::string url, std::string fileName, std::string userPWD)
{
    FILE *fp = fopen(fileName.c_str(), "wb");
    if (!fp)
    {
        std::cout << "fehler aufgetreten bei erstellung der datei" << std::endl;
        return false;
    }

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userPWD.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

    CURLcode result = curl_easy_perform(curl);
    if (result)
    {
        std::cout << "fehler beim download aufgetreten";
        return false;
    }

    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    if (!((responseCode == 200 || responseCode == 201) && result != CURLE_ABORTED_BY_CALLBACK))
    {
        std::cout << "response code: " << responseCode << std::endl;
        return false;
    }

    curl_easy_cleanup(curl);
    fclose(fp);
    return true;
}

std::string getLinkToImage(Onvif camera)
{
    camera.GetProfiles();
    std::string profile = camera.getProfile(0);
    return camera.GetSnapshotUri(profile);
}
std::string getLinkToStream(Onvif camera)
{
    camera.GetProfiles();
    std::string profile = camera.getProfile(0);
    return camera.GetStreamUri(profile);
}

void download10Images(std::string linkToImage, Onvif camera)
{
    while (true)
    {
        if (time(NULL) % 10 == 0)
        {
            break;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
    for (int i = 0; i < 11; i++)
    {
        std::string fileName;
        // std::string hi = "http://10.15.2.201/onvif-cgi/jpg/image.cgi?resolution=1920x1080&compression=90";

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        // time_t sysTime = time(NULL);
        fileName = "downloadedImages/out";
        fileName += std::to_string(i);
        fileName += ".jpg";
        // if (!download_jpeg(hi, fileName, "admin:password"))
        // if (!writeImageToFile(linkToImage, fileName, "admin:password"))
        if (!writeImageToFile(linkToImage, fileName, camera.getUserPWD()))
        {
            std::cout << "download failed" << std::endl;
            return;
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        std::cout << "it took " << timeToFinish << "[ms]"
                  << " to download the image: " << fileName << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 - timeToFinish));
    }
}
void record10Seconds(std::string linkToStream, Onvif camera)
{
    // std::cout << camera.getUserPWD() << std::endl;
    std::string command = "";
    command += "ffmpeg -y -i ";
    // command += "\"";
    command += linkToStream;
    // command += "\"";
    command += " -t 10 -c:v copy out2.mp4";
    std::cout << "hier kommt der kommand: "<< command << std::endl;
    system(command.c_str());
}
void ffmpegPicturesToVid()
{
    system("ffmpeg -y -r 1 -i downloadedImages/out%d.jpg -c:v libx264 -vf \"fps=1,format=yuv420p\" downloadedImages/out.mp4");
}

int main()
{
    // std::string time1 = Onvif::getISO8601DateAndTime(0);
    // Onvif mobotix("10.15.100.200", "admin", "password");
    Onvif axis("10.15.2.201", "seb", "sebseb");

    std::string linkToImage = getLinkToImage(axis);
    std::cout << linkToImage << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::string linkToStream = getLinkToStream(axis);
    std::cout << linkToStream << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    download10Images(linkToImage, axis);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "it took " << timeToFinish << "[ms]"
              << " to finish method" << std::endl;
    // system("cd downloadedImages");
    ffmpegPicturesToVid();
    std::string rtspWithAuth = "";
    rtspWithAuth += "rtsp://";
    rtspWithAuth += axis.getUserPWD();
    rtspWithAuth += "@";

    linkToStream.replace(linkToStream.find("rtsp://"), sizeof("rtsp://") - 1, rtspWithAuth);

    std::cout << linkToStream << std::endl;
    record10Seconds(linkToStream, axis);

    return 0;
}

//   rtsp://10.15.2.201/onvif-media/media.amp?profile=profile_1_h264&sessiontimeout=60&streamtype=unicast
//   ffmpeg -i "rtsp://seb:sebseb@10.15.2.201/onvif-media/media.amp?profile=profile_1_h264&sessiontimeout=60&streamtype=unicast" -t 5 -c:v copy out.mp4