#ifndef MEDIA_IMAGE_IMAGE_H
#define MEDIA_IMAGE_IMAGE_H

#include <curl/curl.h>
#include <iostream>
#include <cstdlib>

class Image
{
private:
    std::string name;
    std::string snapshotURI;
    std::string path;
public:
    Image();
    Image(std::string path, std::string name, std::string snapshotURI);
    void fetchImage();
};

#endif