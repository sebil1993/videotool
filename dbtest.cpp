#include <iostream>
#include "DBLite.h"
#include <boost/filesystem.hpp>

DBLite checkForDB(std::string pathToDatabase)
{
    if (!boost::filesystem::exists(pathToDatabase.c_str()))
    {
        // init db und erstelle eventtypes
        DBLite sqlDB(pathToDatabase.c_str());

        sqlDB.createTable();
        sqlDB.insertEventtypes("Zutritt");

        return sqlDB;
    }
    else
    {
        // std::cout << "database exists" << std::endl;
        DBLite sqlDB(pathToDatabase.c_str());
        // sqlDB.showTable("cameras");
        // sqlDB.closeDB();

        return sqlDB;
    }
}

int main()
{
    DBLite db = checkForDB("storage/database/database.db");

    // db.showTable("cameras");
    // std::string searchQuery = "ipaddress,username,password";
    std::string searchQuery = "*";
    db.searchEntry("cameras",searchQuery,"id","4");

    return 0;
}