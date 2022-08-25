#include <boost/regex.hpp>
#include <string>
#include <iostream>
#include "Onvif/Onvif.h"
#include "DBLite.h"
#include <boost/filesystem.hpp>

bool isValidIp(std::string IP)
{
    boost::regex expr{"^(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[0-9]{1,2})(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[0-9]{1,2})){3}(:((6553[0-5])|(655[0-2][0-9])|(65[0-4][0-9]{2})|(6[0-4][0-9]{3})|([1-5][0-9]{4})|([0-5]{1,5})|([0-9]{1,4})))?$|^$"};
    return boost::regex_match(IP, expr) << '\n';
}

DBLite checkForDB(std::string pathToDatabase)
{
    if (!boost::filesystem::exists(pathToDatabase.c_str()))
    {
        // init db und erstelle eventtypes
        DBLite sqlDB(pathToDatabase.c_str());
        std::cout << "creating database at: " << std::endl;
        sqlDB.createTable();
        sqlDB.insertEventtypes("Zutritt");

        return sqlDB;
    }
    else
    {
        std::cout << "database exists" << std::endl;
        DBLite sqlDB(pathToDatabase.c_str());
        // sqlDB.showTable("cameras");
        // sqlDB.closeDB();

        return sqlDB;
    }
}

// init bereitet nur alles vor, sodass ein prozess gestartet werden kann der dann aufnimmt
int main(int argc, char *argv[])
{
    DBLite db = checkForDB("storage/database/database.db");
    std::string ip_address, username, password;

    // check if ip is given
    if (argc == 1)
    {
        std::cout << "no IP given" << std::endl;

        exit(0);
    }

    // check if valid ip
    if (argc > 1)
    {
        if (isValidIp(argv[1]))
            ip_address = argv[1];
        else
        {
            std::cout << "IP not valid" << std::endl;
            exit(0);
        }
    }
    else
    {
        std::cout << "no input given" << std::endl;
        exit(0);
    }

    // check if ip is in database
    std::vector<std::string> cameraValues = db.searchEntry("cameras", "*", "ipaddress", ip_address);
    if (cameraValues.size() > 0)
    {
        std::cout << "camera in database" << std::endl;
        if (cameraValues[CAM_STREAMURI].size() > 0)
        {
            std::cout << "camera has streamuri" << std::endl;
        }
    }
    else
    {
        std::cout << "camera not found in database" << std::endl;
        // check if username and password are given
        if (argc > 3)
        {
            username = argv[2];
            password = argv[3];
            if (strlen(argv[2]) == 0)
            {
                std::cout << "username empty" << std::endl;
                username = "";
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
        // set authMode and debugMode
        if (argc > 4)
        {
            if (strcmp(argv[4], "-auth") == 0)
            {
                authInHeader = true;
            }
            else if (strcmp(argv[4], "-debug") == 0)
            {
                debug = true;
            }
        }
        if (argc > 5)
        {
            if (strcmp(argv[5], "-debug") == 0)
            {
                debug = true;
            }
            else if (strcmp(argv[5], "-auth") == 0)
            {
                authInHeader = true;
            }
        }

        Onvif camera(ip_address, username, password);
        // initialize camera with given input
        camera.init(authInHeader, debug);
        // check if StreamUri could be created
        if (camera.getStreamUri().size() > 10)
        {
            std::cout << "first check successfull" << std::endl;
        }
        // if not, try other authMode
        else
        {
            std::cout << "could not authenticate with curl, trying with SOAP" << std::endl;
            authInHeader = !authInHeader;
            camera.init(authInHeader, debug);
            // check if StreamUri could be created in other authMode
            if (camera.getStreamUri().size() > 10)
            {
                std::cout << "second check successfull" << std::endl;
            }
            else
            {
                std::cout << "could not authenticate with SOAP" << std::endl;
                std::cout << "aborting..." << std::endl;
                return 0;
            }
        }
        db.insertCameras(ip_address, username, password, camera.getManufacturer(), camera.getModel(), camera.getSerialnumber(), camera.getStreamUri());
    }

    // db.showTable("cameras");

    return 0;
}
