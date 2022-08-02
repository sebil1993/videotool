#ifndef ONVIF_ONVIF_H
#define ONVIF_ONVIF_H
#include <string>

#include <iostream>
#include <vector>
#include <pugixml.hpp>

class Onvif
{
private:
    std::string ipAdress;
    std::string username;
    std::string password;
    std::string streamUri;
    std::string snapshotUri;
    std::vector<std::string> deviceInformation;
    std::vector<std::string> profiles;
    // int deltaTime;
    bool debug;
    bool authInHeader;
    std::string curlRequest(std::string soapMessage);
    pugi::xml_node getResponseBody(std::string curlResponse);
    std::vector<std::string> passwordDigest(std::string password, std::string timestamp);
    std::string getPassword();
    static std::string getISO8601DateAndTime();

public:
    Onvif();
    Onvif(std::string ipAdress, std::string username, std::string password);
    void setIP(std::string ipAdress);
    void setDebugMode(bool enableDebugMode);
    void setAuthInHeader(bool enableAuthInHeader);
    
    std::string getIP();
    std::string getProfile(int i);
    std::string getUser();
    std::string getUserPWD();
    std::string getStreamUri();
    std::string getSnapshotUri();
    std::string getUniqueDeviceName();
    void getAllInfos();

    //Helper Functions
    void init(bool enableAuthInHeader, bool enableDebugMode);
    bool check();
    //ONVIF Functions
    std::string GetSystemDateAndTime();
    std::string GetStreamUri(std::string profile);
    std::string GetSnapshotUri(std::string profile);
    std::vector<std::string> GetProfiles();
    std::vector<std::string> GetDeviceInformation();
};
#endif