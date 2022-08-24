#include <iostream>
#include "DBLite.h"
#include <boost/filesystem.hpp>

int main()
{
    std::string ipAdress = "10.15.100.200";
    std::string username = "admin";
    std::string password = "password";
    std::string timestamp = "2022-08-24T14:45:33.000Z";

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
    }

}