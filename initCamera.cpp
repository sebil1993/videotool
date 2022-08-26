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

bool checkForEntry(DBLite &db, std::string ip_address)
{
    std::vector<std::string> entryValues = db.searchEntry("cameras", "*", "ipaddress", ip_address);
    if (entryValues.size() > 0)
    {
        if (entryValues[CAM_STREAMURI].size() > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

std::vector<std::string> getInputArray(int argc, char *argv[])
{
    std::vector<std::string> inputs;
    std::string ip_address, username, password;

    if (argc == 1)
    {
        std::cout << "no IP given" << std::endl;
    }

    if (argc > 1)
    {
        if (isValidIp(argv[1]))
            inputs.push_back(argv[1]);
        else
        {
            std::cout << "IP not valid" << std::endl;
        }
    }

    if (argc > 3)
    {
        inputs.push_back(argv[2]);
        inputs.push_back(argv[3]);
        // if (strlen(argv[2]) == 0)
        // {
        //     std::cout << "username empty" << std::endl;
        //     inputs.push_back("");
        //     if (strlen(argv[3]) != 0)
        //     {
        //         std::cout << "password is set but username not" << std::endl;
        //     }
        //     inputs.push_back("");
        // }
    }

    return inputs;
}

std::vector<bool> getInputSettings(int argc, char *argv[])
{
    std::vector<bool> settings;
    // authInHeader => false = auth through curl; true = auth with soap
    bool authInHeader = false;
    bool debug = false;
    if (argc > 4)
    {
        if (strcmp(argv[4], "-auth") == 0)
        {
            // std::cout << "auth is true" << std::endl;
            authInHeader = true;
        }
        else if (strcmp(argv[4], "-debug") == 0)
        {
            // std::cout << "debug is true" << std::endl;
            debug = true;
        }
    }
    if (argc > 5)
    {
        if (strcmp(argv[5], "-auth") == 0)
        {
            // std::cout << "auth is true" << std::endl;
            authInHeader = true;
        }
        else if (strcmp(argv[5], "-debug") == 0)
        {
            // std::cout << "debug is true" << std::endl;
            debug = true;
        }
    }

    settings.push_back(authInHeader);
    settings.push_back(debug);
    return settings;
}

void createCameraEntry(DBLite &db, std::vector<std::string> inputs, std::vector<bool> settings)
{
    Onvif camera(inputs[0], inputs[1], inputs[2]);
    camera.init(settings[0], settings[1]);

    if (camera.getStreamUri().size() > 10)
    {
        std::cout << "first check successfull" << std::endl;
    } // if not, try other authMode
    else
    {
        std::cout << "could not authenticate with curl, trying with SOAP" << std::endl;
        settings[0].flip();
        camera.init(settings[0], settings[1]);
        // check if StreamUri could be created in other authMode
        if (camera.getStreamUri().size() > 10)
        {
            std::cout << "second check successfull" << std::endl;
        }
        else
        {
            std::cout << "could not authenticate with SOAP" << std::endl;
            std::cout << "aborting..." << std::endl;
            return;
        }
    }
    std::cout << "creating Entry for " << camera.getIP() << std::endl;
    db.insertCameras(camera.getIP(), camera.getUser(), camera.getPassword(), camera.getManufacturer(), camera.getModel(), camera.getSerialnumber(), camera.getStreamUri());
}

// init bereitet nur alles vor, sodass ein prozess gestartet werden kann der dann aufnimmt

int main(int argc, char *argv[])
{
    DBLite db = checkForDB("storage/database/database.db");
    std::vector<std::string> inputs = getInputArray(argc, argv);

    if (checkForEntry(db, inputs[0]))
    {
        std::cout << "camera with IP in database" << std::endl;
    }
    else
    {
        std::vector<bool> settings = getInputSettings(argc, argv);
        createCameraEntry(db, inputs, settings);
    }

    db.showTable("cameras");
    return 0;

    //     // initialize camera with given input
    //     camera.init(authInHeader, debug);
    //     // check if StreamUri could be created
    //     if (camera.getStreamUri().size() > 10)
    //     {
    //         std::cout << "first check successfull" << std::endl;
    //     }
    //     // if not, try other authMode
    //     else
    //     {
    //         std::cout << "could not authenticate with curl, trying with SOAP" << std::endl;
    //         authInHeader = !authInHeader;
    //         camera.init(authInHeader, debug);
    //         // check if StreamUri could be created in other authMode
    //         if (camera.getStreamUri().size() > 10)
    //         {
    //             std::cout << "second check successfull" << std::endl;
    //         }
    //         else
    //         {
    //             std::cout << "could not authenticate with SOAP" << std::endl;
    //             std::cout << "aborting..." << std::endl;
    //             return 0;
    //         }
    //     }
    //     db.insertCameras(ip_address, username, password, camera.getManufacturer(), camera.getModel(), camera.getSerialnumber(), camera.getStreamUri());
    // }

    // db.showTable("cameras");

    return 0;
}
