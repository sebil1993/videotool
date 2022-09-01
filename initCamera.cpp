// init bereitet nur alles vor
// hierbei wird ip, acc und pw mitgegeben und anhand der ip wird überprüft ob die kamera in
// der datenbank liegt. Sollte das nicht der fall sein, so wird die kamera initialisiert und
// die so erhaltenen werte werden in die datenbank geschrieben. Ansonsten wird beendet.
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
        std::cout << "creating database at: " << pathToDatabase << std::endl;
        sqlDB.createTable();
        sqlDB.insertEventTypes("Zutritt");

        return sqlDB;
    }
    else
    {
        std::cout << "database exists" << std::endl;
        DBLite sqlDB(pathToDatabase.c_str());

        return sqlDB;
    }
}

std::vector<std::string> checkForEntry(DBLite &db, std::string ip_address)
{
    std::vector<std::string> entryValues = db.searchEntry("cameras", "*", "ipaddress", ip_address);
    // if (entryValues.size() > 0)
    // {
    //     if (entryValues[CAM_STREAMURI].size() > 0)
    //     {
    //         return entryValues;
    //     }
    //     else
    //     {
    //         return entryValues;
    //     }
    // }
    return entryValues;
}

std::vector<std::string> getInputArray(int argc, char *argv[])
{
    std::vector<std::string> inputs;
    std::string ip_address, username, password;

    if (argc == 1)
    {
        std::cout << "no IP given" << std::endl;
        exit(0);
    }

    if (argc > 1)
    {
        if (isValidIp(argv[1]))
            inputs.push_back(argv[1]);
        else
        {
            std::cout << "IP not valid" << std::endl;
            exit(0);
        }
    }

    if (argc > 3)
    {
        inputs.push_back(argv[2]);
        inputs.push_back(argv[3]);
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

std::vector<std::string> createCameraEntry(DBLite &db, std::vector<std::string> inputs, std::vector<bool> settings)
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
            exit(0);
        }
    }
    std::cout << "creating Entry for " << camera.getIP() << std::endl;
    return db.insertCameras(camera.getIP(), camera.getUser(), camera.getPassword(), camera.getManufacturer(), camera.getModel(), camera.getSerialnumber(), camera.getStreamUri());
}

int main(int argc, char *argv[])
{
    boost::filesystem::path databasePath = boost::filesystem::current_path();
    databasePath += "/storage/database/database.db";
    
    DBLite db(databasePath.string());
    std::vector<std::string> inputs = getInputArray(argc, argv);
    std::vector<std::string> output = checkForEntry(db, inputs[0]);

    if (output.size() > 0)
    {
        std::cout << "camera found" << std::endl;
    }
    else
    {
        std::vector<bool> settings = getInputSettings(argc, argv);
        output = createCameraEntry(db, inputs, settings);
    }

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-show_table") == 0)
        {
            db.showTable("cameras");
            break;
        }
    }
    //zusätzlich zum db eintrag wird auch der ordner erstellt in dem die ereignisse liegen werden
    std::string startBufferSystemCall = "./startBuffer $IPADDRESS$";
    startBufferSystemCall.replace(startBufferSystemCall.find("$IPADDRESS$"),sizeof("$IPADDRESS$")-1, inputs[0]);
        
    //./startBuffer 10.15.100.200
    
    system(startBufferSystemCall.c_str());
   
    return 0;
}
