#include <iostream>
#include <string>
#include <sqlite3.h>

// https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
// https://videlais.com/2018/12/14/c-with-sqlite3-part-5-encapsulating-database-objects/

using namespace std;

class DBLite
{

private:
    // Pointer to SQLite connection
    sqlite3 *db;

    // Save any error messages
    char *zErrMsg;

    // Save the result of opening the file
    int rc;

    // Saved SQL
    string data;

    // Create a callback function
    static int callback(void *NotUsed, int argc, char **argv, char **azColName)
    {

        // int argc: holds the number of results
        // (array) azColName: holds each column returned
        // (array) argv: holds each value
        for (int i = 0; i < argc; i++)
        {
            // Show column name, value, and newline
            cout << azColName[i] << ": " << argv[i] << endl;
        }

        cout << endl;
        return 0;
    }

    void checkDBErrors()
    {
        if (rc)
        {
            // Show an error message
            cout << "DB Error: " << sqlite3_errmsg(db) << endl;
            closeDB();
        }
    }

public:
    DBLite()
    {
        // Save the result of opening the file
        rc = sqlite3_open("example.db", &db);

        checkDBErrors();
    }
    DBLite(string databaseFileName)
    {
        // Save the result of opening the file
        rc = sqlite3_open(databaseFileName.c_str(), &db);

        checkDBErrors();
    }

    void createTable()
    {
        // Erstes Mal Starten
        data = "CREATE TABLE cameras (ID INTEGER PRIMARY KEY AUTOINCREMENT, ipaddress VARCHAR(255) NOT NULL, username VARCHAR(255) DEFAULT '', password VARCHAR(255) DEFAULT '', manufacturer VARCHAR(255) DEFAULT '', model VARCHAR(255) DEFAULT '', serialnumber VARCHAR(255) DEFAULT '', streamuri VARCHAR(255) DEFAULT '', snapshoturi VARCHAR(255) DEFAULT '');";
        rc = sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
        data = "CREATE TABLE eventtypes (ID INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR(255) NOT NULL);";
        rc = sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
        data = "CREATE TABLE events (ID INTEGER PRIMARY KEY AUTOINCREMENT, timestamp datetime default current_timestamp, camera_id int, eventtype_id int, foreign key (camera_id) references cameras (id), foreign key (eventtype_id) references eventtypes (id));";
        rc = sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }
    //insert cameras 
    void insertCameras(string IPAddress)
    {
        data = "INSERT INTO CAMERAS ('ipaddress') VALUES ('$IPAddress$');";
        data.replace(data.find("$IPAddress$"), sizeof("$IPAddress$") - 1, IPAddress.c_str());
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }
    void insertCameras(string IPAddress, string username, string password)
    {
        data = "INSERT INTO CAMERAS ('ipaddress', 'username', 'password',) VALUES ('$IPAddress$','$username$','$password$');";
        data.replace(data.find("$IPAddress$"), sizeof("$IPAddress$") - 1, IPAddress.c_str());
        data.replace(data.find("$username$"), sizeof("$username$") - 1, username.c_str());
        data.replace(data.find("$password$"), sizeof("$password$") - 1, password.c_str());
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }
    void insertCameras(string IPAddress, string username, string password, string manufacturer, string model, string serialnumber, string streamuri)
    {
        data = "INSERT INTO CAMERAS ('ipaddress', 'username', 'password', 'manufacturer', 'model', 'serialnumber', 'streamuri') VALUES ('$IPAddress$','$username$','$password$','$manufacturer$','$model$','$serialnumber$','$streamuri$');";
        data.replace(data.find("$IPAddress$"), sizeof("$IPAddress$") - 1, IPAddress.c_str());
        data.replace(data.find("$username$"), sizeof("$username$") - 1, username.c_str());
        data.replace(data.find("$password$"), sizeof("$password$") - 1, password.c_str());
        data.replace(data.find("$manufacturer$"), sizeof("$manufacturer$") - 1, manufacturer.c_str());
        data.replace(data.find("$model$"), sizeof("$model$") - 1, model.c_str());
        data.replace(data.find("$serialnumber$"), sizeof("$serialnumber$") - 1, serialnumber.c_str());
        data.replace(data.find("$streamuri$"), sizeof("$streamuri$") - 1, streamuri.c_str());
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    void insertEventtypes(string eventName)
    {
        data = "INSERT INTO eventtypes(name) VALUES('$eventName$');";
        data.replace(data.find("$eventName$"), sizeof("$eventName$") - 1, eventName.c_str());
        std::cout << data << endl;
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }
    
    void insertEvent(int camera_id, int eventtype_id)
    {
        data = "INSERT INTO events(camera_id, eventtype_id) VALUES('$camera_id$', '$eventtype_id$');";
        data.replace(data.find("$camera_id$"), sizeof("$camera_id$") - 1, to_string(camera_id).c_str());
        data.replace(data.find("$eventtype_id$"), sizeof("$eventtype_id$") - 1, to_string(eventtype_id).c_str());
        std::cout << data << endl;
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }
    void insertEvent(string timestamp, int camera_id, int eventtype_id)
    {
        data = "INSERT INTO events(timestamp, camera_id, eventtype_id) VALUES('$timestamp$', '$camera_id$', '$eventtype_id$');";
        data.replace(data.find("$timestamp$"), sizeof("$timestamp$") - 1, timestamp.c_str());
        data.replace(data.find("$camera_id$"), sizeof("$camera_id$") - 1, to_string(camera_id).c_str());
        data.replace(data.find("$eventtype_id$"), sizeof("$eventtype_id$") - 1, to_string(eventtype_id).c_str());
        std::cout << data << endl;
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    void showTable(string table)
    {
        data = "SELECT * FROM $TABLE$;";
        data.replace(data.find("$TABLE$"), sizeof("$TABLE$") - 1, table.c_str());
        rc = sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    //entweder id oder ip mitgeben
    void deleteCamera(int ID)
    {
        data = "DELETE FROM CAMERAS WHERE ID = '$ID$';";
        
        data.replace(data.find("$ID$"), sizeof("$ID$") - 1, to_string(ID).c_str());

        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    void closeDB()
    {
        sqlite3_close(db);
    }
};
