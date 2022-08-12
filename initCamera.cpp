#include <boost/regex.hpp>
#include <string>
#include <iostream>
#include "Onvif/Onvif.h"
#include "DBLite.h"

bool isValidIp(std::string IP)
{
    boost::regex expr{"^(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[0-9]{1,2})(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[0-9]{1,2})){3}(:((6553[0-5])|(655[0-2][0-9])|(65[0-4][0-9]{2})|(6[0-4][0-9]{3})|([1-5][0-9]{4})|([0-5]{1,5})|([0-9]{1,4})))?$|^$"};
    return boost::regex_match(IP, expr) << '\n';
}

int main(int argc, char *argv[])
{
    std::string ip_address, username, password;
    if (argc == 1)
    {
        std::cout << "no IP given" << std::endl;

        exit(0);
    }

    if (argc > 1)
    {
        if (isValidIp(argv[1]))
            ip_address = argv[1];
        else
        {
            std::cout << "IP check failed" << std::endl;
            exit(0);
        }
    }
    else
    {
        std::cout << "no input given" << std::endl;
        exit(0);
    }
    if (argc > 3)
    {
        username = argv[2];
        password = argv[3];
        if (strlen(argv[2]) == 0)
        {
            username = "";
            std::cout << "username empty" << std::endl;
            if (strlen(argv[3]) != 0)
            {
                std::cout << "password is set but username not" << std::endl;
                exit(0);
            }
            password = "";
        }
    }
    bool authInHeader = false;
    bool debug = false;
    if (argc > 4)
    {
        if (strcmp(argv[4], "true") == 0)
        {
            authInHeader = true;
        }

        if (argc > 5)
        {
            if (strcmp(argv[5], "true") == 0)
                debug = true;
        }
    }
    Onvif camera(ip_address, username, password);

    camera.init(authInHeader, debug);

    if (camera.getStreamUri().size() > 10)
    {
        std::cout << "first check successfull" << std::endl;
    }
    else
    {
        authInHeader = !authInHeader;
        camera.init(authInHeader, debug);
    }

    if (camera.getStreamUri().size() > 10)
    {
        std::cout << "second check successfull" << std::endl;

        camera.getAllInfos();

        // DBLite sqlDB("database.db");
        // sqlDB.createTable();

        // sqlDB.insertData("2", camera.getIP(), camera.getUser(), camera.getPassword(), camera.getStreamUri());

        // sqlDB.showTable();
    }
    // DBLite sqlDB("database.db");
    // sqlDB.createTable();

    // sqlDB.insertData("2", camera.getIP(), camera.getUser(), camera.getPassword(), camera.getStreamUri());
    // sqlDB.showTable();

    return 0;
}
