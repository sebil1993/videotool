#include "Onvif.h"

#include "Onvif.h"
#include "sha1.hpp"
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <vector>
#include <pugixml.hpp>

Onvif::Onvif()
{
    this->ipAdress = "";
    this->username = "";
    this->password = "";
    this->deltaTime = 5;
    this->profiles = {};
    this->streamUri = "";
    this->snapshotUri = "";
    this->deviceInformation = {};
}

Onvif::Onvif(std::string ipAdress, std::string username, std::string password)
{
    this->ipAdress = ipAdress;
    this->username = username;
    this->password = password;
}
void Onvif::setIP(std::string ipAdress)
{
    this->ipAdress = ipAdress;
}
std::string Onvif::getIP()
{
    return this->ipAdress;
}
std::string Onvif::getSnapshotUri()
{
    return this->snapshotUri;
}
std::string Onvif::getPassword()
{
    return this->password;
}

std::string Onvif::getUser()
{
    return this->username;
}
std::string Onvif::getUserPWD()
{
    std::string UserPWD;
    UserPWD += this->getUser();
    UserPWD += ":";
    UserPWD += this->getPassword();
    return UserPWD;
}

std::string Onvif::getProfile(int i)
{
    if (this->profiles.size() <= 0)
    {
        std::cout << "no profiles, starting GetProfiles() ..." << std::endl;
        this->GetProfiles();
    }
    if (i >= this->profiles.size())
    {
        i = i % this->profiles.size();
    }
    return this->profiles[i];
}

// REFACTOR
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string Onvif::getISO8601DateAndTime(int deltaTime = 0)
{
    time_t now;
    struct tm ts;
    char buf[28];

    time(&now);
    // now -= deltaTime;
    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.000Z", &ts);
    // std::cout << mktime(&ts) << std::endl; => GIBT ZEIT ALAS ZEITSTEMPEL ZÜRÜCK
    std::string ISO8601(buf);
    // std::cout << ISO8601 << std::endl;
    return ISO8601;
}
static std::string base64_encode(const std::string &in)
{
    std::string out;

    int val = 0, valb = -6;
    for (char c : in)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');
    return out;
}
std::vector<std::string> Onvif::passwordDigest(std::string password, std::string timestamp)
{
    srand(time(NULL));
    std::string nonce = std::to_string(rand());
    std::string passdigest;

    passdigest += nonce;
    passdigest += timestamp;
    passdigest += password;

    char hex[SHA1_HEX_SIZE];
    char base64[SHA1_BASE64_SIZE];

    sha1(passdigest.c_str())
        .finalize()
        .print_hex(hex)
        .print_base64(base64);

    std::vector<std::string> digestArray;
    digestArray.push_back(getUser());
    digestArray.push_back(base64);
    digestArray.push_back(base64_encode(nonce));
    digestArray.push_back(timestamp);

    return digestArray;
}

std::string Onvif::curlRequest(std::string soapMessage)
{
    struct curl_slist *header = NULL;
    int soapMessageLength = soapMessage.length();

    std::string contentLengthHeader = "Conent-Length: ";
    contentLengthHeader += std::to_string(soapMessageLength);
    std::string host = "Host: ";
    host += this->ipAdress;

    header = curl_slist_append(header, "Content-type: application/soap+xml; charset=utf-8");
    header = curl_slist_append(header, contentLengthHeader.c_str());
    header = curl_slist_append(header, host.c_str());

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
        std::string url;
        std::string readBuffer;

        url.reserve(55);
        url += "http://";
        url += this->ipAdress;
        url += "/onvif/device_service";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, soapMessage.c_str());

        curl_easy_setopt(curl, CURLOPT_VERBOSE, false);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        return readBuffer;
    }
    curl_easy_cleanup(curl);
    return "error couldn't curl";
}
pugi::xml_node Onvif::getResponseBody(std::string curlResponse)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(curlResponse.c_str());
    if (result == 1)
    {
        pugi::xml_node responseBody = doc.first_child().child("SOAP-ENV:Body").first_child();
        return responseBody;
    }
    else
    {
        pugi::xml_node x;
        return x;
    }
}

std::string Onvif::GetSystemDateAndTime()
{
    std::string data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\"><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><GetSystemDateAndTime xmlns=\"http://www.onvif.org/ver10/device/wsdl\"/></s:Body></s:Envelope>";
    std::string curlResponse = curlRequest(data);

    pugi::xml_node getSystemDateAndTimeResponse = getResponseBody(curlResponse);

    pugi::xml_node UTCTimeNode = getSystemDateAndTimeResponse.first_child().child("tt:UTCDateTime").child("tt:Time");
    pugi::xml_node UTCDateNode = getSystemDateAndTimeResponse.first_child().child("tt:UTCDateTime").child("tt:Date");

    time_t rawtime;
    int year, month, day;
    int hour, minute, second;

    year = UTCDateNode.child("tt:Year").text().as_int();
    month = UTCDateNode.child("tt:Month").text().as_int();
    day = UTCDateNode.child("tt:Day").text().as_int();
    hour = UTCTimeNode.child("tt:Hour").text().as_int();
    minute = UTCTimeNode.child("tt:Minute").text().as_int();
    second = UTCTimeNode.child("tt:Second").text().as_int();

    struct tm *timeinfo;
    timeinfo = gmtime(&rawtime);

    timeinfo->tm_year = year - 1900;
    timeinfo->tm_mon = month - 1;
    timeinfo->tm_mday = day;
    timeinfo->tm_hour = hour;
    timeinfo->tm_min = minute;
    timeinfo->tm_sec = second + 5;

    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.000Z", timeinfo);
    std::string ISO8601(buf);

    // TODO an dieser stelle wahrscheinlich noch die this->deltatime setzen (lokale zeit - gerätezeit)
    // damit ich nicht jedes Mal nachfragen muss wie spät es ist und als fehlerhandling könnte man einbauen
    // dass er es doch tut, wenn iwas nicht stimmt
    return ISO8601;
}
std::vector<std::string> Onvif::GetProfiles()
{
    std::vector<std::string> pwdigest = passwordDigest(this->password, this->GetSystemDateAndTime());
    std::vector<std::string> profiles;
    std::string data = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><s:Header><wsse:Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>$$USERNAME$$</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">$$PASSWORD$$</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">$$NONCE$$</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">$$CREATED$$</Created></UsernameToken></wsse:Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><GetProfiles xmlns=\"http://www.onvif.org/ver10/media/wsdl\"/></s:Body></s:Envelope>";
    data.replace(data.find("$$USERNAME$$"), sizeof("$$USERNAME$$") - 1, pwdigest[0]);
    data.replace(data.find("$$PASSWORD$$"), sizeof("$$PASSWORD$$") - 1, pwdigest[1]);
    data.replace(data.find("$$NONCE$$"), sizeof("$$NONCE$$") - 1, pwdigest[2]);
    data.replace(data.find("$$CREATED$$"), sizeof("$$CREATED$$") - 1, pwdigest[3]);

    std::string curlResponse = curlRequest(data);
    pugi::xml_node getProfilesResponse = getResponseBody(curlResponse);
    for (pugi::xml_node profile : getProfilesResponse)
    {
        this->profiles.push_back(profile.attribute("token").value());
    }
    return this->profiles;
}

std::string Onvif::GetStreamUri(std::string profile)
{
    std::vector<std::string> pwdigest = passwordDigest(this->password, this->GetSystemDateAndTime());
    std::string data = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><s:Header><wsse:Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>$$USERNAME$$</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">$$PASSWORD$$</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">$$NONCE$$</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">$$CREATED$$</Created></UsernameToken></wsse:Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><GetStreamUri xmlns=\"http://www.onvif.org/ver10/media/wsdl\"><StreamSetup><Stream xmlns=\"http://www.onvif.org/ver10/schema\">RTP-Unicast</Stream><Transport xmlns=\"http://www.onvif.org/ver10/schema\"><Protocol>RTSP</Protocol></Transport></StreamSetup><ProfileToken>$$PROFILETOKEN$$</ProfileToken></GetStreamUri></s:Body></s:Envelope>";

    data.replace(data.find("$$USERNAME$$"), sizeof("$$USERNAME$$") - 1, pwdigest[0]);
    data.replace(data.find("$$PASSWORD$$"), sizeof("$$PASSWORD$$") - 1, pwdigest[1]);
    data.replace(data.find("$$NONCE$$"), sizeof("$$NONCE$$") - 1, pwdigest[2]);
    data.replace(data.find("$$CREATED$$"), sizeof("$$CREATED$$") - 1, pwdigest[3]);
    data.replace(data.find("$$PROFILETOKEN$$"), sizeof("$$PROFILETOKEN$$") - 1, profile);

    std::string curlResponse = curlRequest(data);
    pugi::xml_node getStreamUriResponse = getResponseBody(curlResponse);

    this->streamUri = getStreamUriResponse.first_child().child("tt:Uri").text().as_string();
    return this->streamUri;
}
std::string Onvif::GetSnapshotUri(std::string profile)
{
    std::vector<std::string> pwdigest = passwordDigest(this->password, this->GetSystemDateAndTime());
    std::string data = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><s:Header><wsse:Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>$$USERNAME$$</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">$$PASSWORD$$</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">$$NONCE$$</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">$$CREATED$$</Created></UsernameToken></wsse:Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><GetSnapshotUri xmlns=\"http://www.onvif.org/ver10/media/wsdl\"><ProfileToken>$$PROFILETOKEN$$</ProfileToken></GetSnapshotUri></s:Body></s:Envelope>";

    data.replace(data.find("$$USERNAME$$"), sizeof("$$USERNAME$$") - 1, pwdigest[0]);
    data.replace(data.find("$$PASSWORD$$"), sizeof("$$PASSWORD$$") - 1, pwdigest[1]);
    data.replace(data.find("$$NONCE$$"), sizeof("$$NONCE$$") - 1, pwdigest[2]);
    data.replace(data.find("$$CREATED$$"), sizeof("$$CREATED$$") - 1, pwdigest[3]);
    data.replace(data.find("$$PROFILETOKEN$$"), sizeof("$$PROFILETOKEN$$") - 1, profile);

    std::string curlResponse = curlRequest(data);
    pugi::xml_node getSnapshotUriResponse = getResponseBody(curlResponse);
    this->snapshotUri = getSnapshotUriResponse.first_child().child("tt:Uri").text().as_string();
    // this->snapshotUri += "\n";
    return this->snapshotUri;
}
std::vector<std::string> Onvif::GetDeviceInformation()
{
    std::vector<std::string> pwdigest = passwordDigest(this->password, this->GetSystemDateAndTime());
    std::string data = "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><s:Header><wsse:Security s:mustUnderstand=\"1\" xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\"><UsernameToken><Username>$$USERNAME$$</Username><Password Type=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest\">$$PASSWORD$$</Password><Nonce EncodingType=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary\">$$NONCE$$</Nonce><Created xmlns=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\">$$CREATED$$</Created></UsernameToken></wsse:Security></s:Header><s:Body xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"><GetDeviceInformation xmlns=\"http://www.onvif.org/ver10/device/wsdl\"/></s:Body></s:Envelope>";

    data.replace(data.find("$$USERNAME$$"), sizeof("$$USERNAME$$") - 1, pwdigest[0]);
    data.replace(data.find("$$PASSWORD$$"), sizeof("$$PASSWORD$$") - 1, pwdigest[1]);
    data.replace(data.find("$$NONCE$$"), sizeof("$$NONCE$$") - 1, pwdigest[2]);
    data.replace(data.find("$$CREATED$$"), sizeof("$$CREATED$$") - 1, pwdigest[3]);

    std::string curlResponse = curlRequest(data);
    pugi::xml_node getDeviceInformationResponse = getResponseBody(curlResponse);

    this->deviceInformation.push_back(getDeviceInformationResponse.child("tds:Manufacturer").text().as_string());
    this->deviceInformation.push_back(getDeviceInformationResponse.child("tds:Model").text().as_string());

    return this->deviceInformation;
}