#ifndef ONVIF_ONVIF_H
#define ONVIF_ONVIF_H
#include <string>

#include <iostream>
#include <vector>
#include <pugixml.hpp>

class Onvif
{
private:
    // struct Profile
    // {
    //     // std::string name;
    //     std::string token;
    //     std::string snapshotUri;
    //     std::string streamUri;
    //     // struct videoEncoderConfiguration
    //     // {
    //     //     std::string encoderType;
    //     //     std::string quality;
    //     //     struct resolution
    //     //     {
    //     //         std::string width;
    //     //         std::string height;
    //     //     };
    //     // };
    // };
    std::string ipAdress;
    std::string username;
    std::string password;
    std::string streamUri;
    std::string snapshotUri;
    std::vector<std::string> deviceInformation;
    std::vector<std::string> profiles;
    int deltaTime;
    std::string curlRequest(std::string soapMessage);
    pugi::xml_node getResponseBody(std::string curlResponse);
    std::vector<std::string> passwordDigest(std::string password, std::string timestamp);
    std::string getPassword();

public:
    Onvif();
    Onvif(std::string ipAdress, std::string username, std::string password);
    void setIP(std::string ipAdress);
    std::string getIP();
    std::string getProfile(int i);
    std::string getUser();
    std::string getUserPWD();
    std::string getSnapshotUri();
    std::string GetSystemDateAndTime();
    std::string GetStreamUri(std::string profile);
    std::string GetSnapshotUri(std::string profile);
    static std::string getISO8601DateAndTime(int deltaTime);
    std::vector<std::string> GetProfiles();
    std::vector<std::string> GetDeviceInformation();
};
#endif