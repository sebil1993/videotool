#include "Onvif/Onvif.h"
#include <stdio.h>  /* printf */
#include <time.h>   /* time_t, struct tm, difftime, time, mktime */
#include <iostream> // std::cout, std::endl
#include <thread>   // std::this_thread::sleep_for
#include <chrono>   // std::chrono::seconds
#include <curl/curl.h>

#include <deque>
#include <cstdlib>
#include <fstream>

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
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

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

    // curl_easy_cleanup(curl);
    fclose(fp);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 - timeToFinish));
    return true;
}

std::string GetStdoutFromCommand(std::string cmd)
{

    std::string data;
    FILE *stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");

    if (stream)
    {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL)
                data.append(buffer);
        pclose(stream);
    }
    return data;
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

void downloadImage(std::string linkToImage, Onvif camera)
{

    // std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::string fileName;
    time_t sysTime = time(NULL);
    fileName = "downloadedImages/AXIS_";
    fileName += std::to_string(sysTime);
    fileName += ".jpg";

    if (!writeImageToFile(linkToImage, fileName, camera.getUserPWD()))
    {
        std::cout << "download failed" << std::endl;
        return;
    }
    // std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    // long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    // std::cout << sysTime << "it took " << timeToFinish << "[ms]"
    //           << " to download the image: " << fileName << std::endl;
    // std::chrono::steady_clock::time_point end2 = std::chrono::steady_clock::now();
    // long long timeToFinish2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - begin).count();

    // std::this_thread::sleep_for(std::chrono::milliseconds(1000 - timeToFinish2));
}

void ffmpegPicturesToVid()
{
    system("ffmpeg -hide_banner -y -r 1 -i downloadedImages/out%d.jpg -c:v libx264 -vf \"fps=1,format=yuv420p\" downloadedImages/out.mp4");
}
void record10Seconds(std::string linkToStream, Onvif camera)
{
    std::string command = "";
    command += "ffmpeg -y -i ";
    command += linkToStream;
    command += " -t 10 -c:v copy out2.mp4";
    system(command.c_str());
}

int countImages(std::string path)
{
    path += "/*.jpg ";
    std::string command = "ls -l ";
    command += path;
    command += "| wc -l";
    int imageCount = std::stoi(GetStdoutFromCommand(command.c_str()));

    return imageCount;
}
void deleteFirstImage()
{
    std::string firstFile = GetStdoutFromCommand("ls downloadedImages/*.jpg| head -1");
    firstFile.pop_back();
    std::remove(firstFile.c_str());
}
int main()
{
    Onvif axis("10.15.2.201", "seb", "sebseb");

    axis.GetProfiles();
    std::string linkToImage = getLinkToImage(axis);

    while (true)
    {

        if (countImages("downloadedImages/") < 10)
        {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

            downloadImage(linkToImage, axis);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

            std::cout << "it took " << timeToFinish << "[ms]"
                      << " to download the image " << std::endl;
        }
        else
            break;
    }
    return 0;
}

/* FÜR SPÄTER

    // std::remove("downloadedImages/.jpg");
    // countImages("hi");
    // std::deque<Onvif> onvif_array;
    // Onvif axis("10.15.2.201", "seb", "sebseb");

    // onvif_array.push_back(axis);
    // // onvif_array.push_back(mobotix);

    // std::cout << onvif_array[0].getProfile(0) << std::endl;
    // // std::cout << onvif_array[1].getProfile(0) << std::endl;

    // std::string linkToImage = getLinkToImage(onvif_array[0]);
    // // std::string linkToImage2 = getLinkToImage(onvif_array[1]);
    // std::cout << linkToImage << std::endl;
    // // std::cout << linkToImage2 << std::endl;

    // // std::string linkToStream = getLinkToStream(onvif_array[0]);
    // // int retryCount = 0;
    // while (true)
    // {
    //     // downloadImage(linkToImage,axis);

    //     // retryCount += 1;
    //     // if (retryCount > 10)
    //     // {

    //     //     std::cout << "hat nicht geklappt" << std::endl;
    //     //     linkToStream = "couldnt perform operation";
    //     // }
    //     // std::cout << "retry" << std::endl;
    //     // std::string linkToStream = getLinkToStream(onvif_array[0]);
    //     // std::cout << "link to stream empty? => "<< linkToStream.empty() << std::endl;

    //     // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    // }
    // // std::cout << linkToStream << std::endl;
    // // std::string linkToStream2 = getLinkToStream(onvif_array[1]);
    // // while (linkToStream.empty())
    // // {
    // //     // retry
    // //     std::cout << "retrying ..." << std::endl;
    // //     std::string linkToStream = getLinkToStream(onvif_array[0]);
    // //     std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    // // }
    // // std::cout << linkToStream << std::endl;
    // // // std::cout << linkToStream2 << std::endl;
    // // std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    // // download10Images(linkToImage, onvif_array[0]);
    // // // download10Images(linkToImage, onvif_array[1]);

    // // std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    // // long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    // // std::cout << "it took " << timeToFinish << "[ms]"
    // //           << " to finish method" << std::endl;
    // // // system("cd downloadedImages");
    // // // ffmpegPicturesToVid();
    // // std::string rtspWithAuth = "";
    // // rtspWithAuth += "rtsp://";
    // // rtspWithAuth += axis.getUserPWD();
    // // rtspWithAuth += "@";

    // // linkToStream.replace(linkToStream.find("rtsp://"), sizeof("rtsp://") - 1, rtspWithAuth);

    // // std::cout << linkToStream << std::endl;
    // // // record10Seconds(linkToStream, axis);

    return 0;
}

//   rtsp://10.15.2.201/onvif-media/media.amp?profile=profile_1_h264&sessiontimeout=60&streamtype=unicast
//   ffmpeg -i "rtsp://seb:sebseb@10.15.2.201/onvif-media/media.amp?profile=profile_1_h264&sessiontimeout=60&streamtype=unicast" -t 5 -c:v copy out.mp4


*/