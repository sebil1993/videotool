#include "Video.h"

void record10Seconds(std::string linkToStream, std::string path, std::string fileName)
{
    std::string command = "ffmpeg -y -i '";
    command += linkToStream;
    command += "' -t 10 -c:v copy ";
    command += path;
    command += "/";
    command += fileName;
    std::cout << command << std::endl;
    system(command.c_str());
}
