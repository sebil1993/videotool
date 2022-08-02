#include "Image.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/path.hpp>

#include "../../Onvif/Onvif.h"
#include <thread>
#include <chrono>

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
        std::cout << "Datei konnte nicht erstellt werden" << std::endl;
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
        std::cout << "Fehler beim Download aufgetreten";
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
void downloadImage(boost::filesystem::path path, std::string linkToImage)
{
    if (!boost::filesystem::exists(path))
    {
        boost::filesystem::create_directories(path);
    }
    std::string fileName;
    time_t sysTime = time(NULL);
    fileName = path.c_str();
    fileName += '/';
    fileName += std::to_string(sysTime);
    fileName += ".jpg";
    std::cout << fileName << std::endl;
    if (!writeImageToFile(linkToImage, fileName, "seb:sebseb"))
    {
        std::cout << "download failed" << std::endl;
        return;
    }
}

void ffmpegPicturesToVid(std::string path, std::string fileName)
{
    std::string command = "ffmpeg -y -r 1 -pattern_type glob -i '";
    command += path;
    command += "/*.jpg' -vf fps=1 -c:v libx264 -crf 40 ";
    command += path;
    command += "/";
    command += fileName;
    std::cout << command << std::endl;
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

void signalHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    std::cout << "starting recording process" << std::endl;
    // cleanup and close up stuff here
    // terminate program

    exit(signum);
}

int main()
{
    // register signal SIGINT and signal handler
    Onvif axis("10.15.2.201", "seb", "sebseb");
    axis.init(false, false);
    axis.getAllInfos();
    int* sig = 0;

    signal(SIGINT, signalHandler);

    auto path = boost::filesystem::current_path();

    path += "/storage/cameras/";
    path += axis.getUniqueDeviceName();

    std::string snapshoturi = axis.getSnapshotUri();
    while (1)
    {
        downloadImage(path, snapshoturi);
        std::cout << "Going to sleep...." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}

/*
int initCamera(std::string ip, std::string acc, std::string pw)
{
    int counter = 0;
    long long allTogether = 0;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (int i = 0; i < 10; i++)
    {
        std::cout << i << " loop =>" << std::endl;
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        // Onvif axis("10.15.2.201", "seb", "sebseb");
        Onvif camera(ip.c_str(), acc.c_str(), pw.c_str());
        // axis.getAllInfos();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        counter++;
        if (i == 9)
        {
            camera.getAllInfos();
        }
        long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        std::cout << "[IT TOOK " << timeToFinish << "ms to finish the LOOP]" << std::endl;

        if (!(timeToFinish > 1000))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 - timeToFinish));
        }

        allTogether += timeToFinish;
    }
    std::cout << "average time => " << allTogether / counter << std::endl;

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout << "[IT TOOK " << timeToFinish << "ms to finish the MAIN]" << std::endl;
    FÜR Download bilder
        // axis.GetProfiles();
        // std::string linkToImage = getLinkToImage(axis);

        // // while (true)
        // // {

        // //     if (countImages("downloadedImages/") < 61)
        // //     {
        // //         downloadImage(linkToImage, axis);
        // //     }
        // //     else
        // //         break;
        // // }
        // // ffmpegPicturesToVid("downloadedImages","axisvideo.mp4");

        // // std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        // std::string linkToStream = getLinkToStream(axis);

        // // std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        // // long long timeToFinish = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

        // // std::cout << timeToFinish << std::endl;
        // std::cout << linkToImage << std::endl << linkToStream << std::endl;
        // std::string rtspWithAuth = "";
        // rtspWithAuth += "rtsp://";
        // rtspWithAuth += axis.getUserPWD();
        // rtspWithAuth += "@";

        // linkToStream.replace(linkToStream.find("rtsp://"), sizeof("rtsp://") - 1, rtspWithAuth);

        // record10Seconds(linkToStream, axis, "downloadedImages","axisvideo2.mp4");

        // std::cout << " done " << std::endl;

        // system("ffmpeg -")

    return 0;
}*/
