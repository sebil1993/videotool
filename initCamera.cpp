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
        boost::filesystem::path path = pathToDatabase;
        path.remove_filename();
        if (!boost::filesystem::exists(path))
        {
            boost::filesystem::create_directories(path);
        }
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

    return entryValues;
}

std::vector<std::string> getInputCredentials(int argc, char *argv[])
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
            authInHeader = true;
        }
        else if (strcmp(argv[4], "-debug") == 0)
        {
            debug = true;
        }
    }
    if (argc > 5)
    {
        if (strcmp(argv[5], "-auth") == 0)
        {
            authInHeader = true;
        }
        else if (strcmp(argv[5], "-debug") == 0)
        {
            debug = true;
        }
    }

    settings.push_back(authInHeader);
    settings.push_back(debug);
    return settings;
}

boost::filesystem::path checkOrCreateDirectory(std::vector<std::string> camera)
{
    std::string nameOfFolder = "";
    nameOfFolder += camera[CAM_MANUFACTURER];
    nameOfFolder += '_';
    nameOfFolder += camera[CAM_MODEL];
    nameOfFolder += '_';
    nameOfFolder += camera[CAM_SERIALNUMBER];
    while (true)
    {
        if (nameOfFolder.find(" ") == std::string::npos)
            break;
        nameOfFolder.replace(nameOfFolder.find(" "), sizeof(" ") - 1, "_");
    }

    boost::filesystem::path path = boost::filesystem::current_path();
    path += "/storage/cameras/";
    path += nameOfFolder;

    if (!boost::filesystem::exists(path))
    {
        std::cout << "creating directory " << nameOfFolder << " for " << camera[CAM_IPADDRESS] << std::endl;
        boost::filesystem::create_directories(path);
    }

    return path;
}

std::vector<std::string> createCameraEntry(DBLite &db, std::vector<std::string> inputs, std::vector<bool> settings)
{
    Onvif camera(inputs[0], inputs[1], inputs[2]);
    std::cout <<"creating camera entry" << std::endl;
    //problem ist dass wenn die ip viable ist. es gibt keine abbruch bedingung fürs curl
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
    std::vector<std::string> cameraEntry = db.insertCameras(camera.getIP(), camera.getUser(), camera.getPassword(), camera.getManufacturer(), camera.getModel(), camera.getSerialnumber(), camera.getStreamUri());
    std::cout << "creating Entry for " << cameraEntry[CAM_IPADDRESS] << std::endl;
    checkOrCreateDirectory(cameraEntry);

    return cameraEntry;
}

int main(int argc, char *argv[])
{
    boost::filesystem::path databasePath = boost::filesystem::current_path();
    databasePath += "/storage/database/database.db";
    DBLite db = checkForDB(databasePath.string());

    std::vector<std::string> inputs = getInputCredentials(argc, argv);

    std::vector<std::string> camera = checkForEntry(db, inputs[0]);
    if (camera.size() > 0)
    {
        std::cout << "camera found" << std::endl;
    }
    else
    {
        std::cout << "no camera found" << std::endl;
        std::vector<bool> settings = getInputSettings(argc, argv);
        camera = createCameraEntry(db, inputs, settings);
    }

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-show_table") == 0)
        {
            db.showTable("cameras");
            break;
        }
    }

    return 0;
}
