#include <iostream>
#include "DBLite.h"
#include <boost/filesystem.hpp>

int main()
{
    std::string ipAdress;
    std::string username;
    std::string password;

    if (!boost::filesystem::exists("storage/database/database.db"))
    {
        // init db und erstelle eventtypes
        DBLite sqlDB("storage/database/database.db");

        sqlDB.createTable();
        sqlDB.insertEventtypes("Zutritt");
    }
    else
    {
        std::cout << "database exists" << std::endl;
        DBLite sqlDB("storage/database/database.db");
        sqlDB.showTable("cameras");
        sqlDB.closeDB();
    }

    return 0;
}