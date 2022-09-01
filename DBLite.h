#include <iostream>
#include <string>
#include <sqlite3.h>
#include <vector>

// https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
// https://videlais.com/2018/12/14/c-with-sqlite3-part-5-encapsulating-database-objects/
// https://www.proggen.org/doku.php?id=dbs:sqlite:libsqlite3:communicate:sqlite3_prepared
using namespace std;

#define CAM_IPADDRESS 1
#define CAM_USERNAME 2
#define CAM_PASSWORD 3
#define CAM_MANUFACTURER 4
#define CAM_MODEL 5
#define CAM_SERIALNUMBER 6
#define CAM_STREAMURI 7
#define CAM_SNAPSHOTURI 8

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
        for (int i = 0; i < argc; i++)
        {
            cout << azColName[i] << ": " << argv[i] << endl;
        }
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
        data = "CREATE TABLE events (ID INTEGER PRIMARY KEY AUTOINCREMENT, timestamp datetime default current_timestamp, camera_id int, eventtype_id int, filepath varchar(255) default '',foreign key (camera_id) references cameras (id), foreign key (eventtype_id) references eventtypes (id));";
        rc = sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }
    // insert cameras
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
    std::vector<std::string> insertCameras(string IPAddress, string username, string password, string manufacturer, string model, string serialnumber, string streamuri)
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

        return this->searchEntry("cameras", "*", "ipaddress", IPAddress.c_str());
    }

    void insertEventTypes(std::string eventName)
    {
        data = "INSERT INTO eventtypes(name) VALUES('$eventName$');";
        data.replace(data.find("$eventName$"), sizeof("$eventName$") - 1, eventName.c_str());
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    int insertEvent(int camera_id, int eventtype_id)
    {
        data = "INSERT INTO events(timestamp, camera_id, eventtype_id) VALUES(datetime('now', 'localtime'), '$camera_id$', '$eventtype_id$');";
        data.replace(data.find("$camera_id$"), sizeof("$camera_id$") - 1, to_string(camera_id).c_str());
        data.replace(data.find("$eventtype_id$"), sizeof("$eventtype_id$") - 1, to_string(eventtype_id).c_str());
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);

        int event_id = sqlite3_last_insert_rowid(db);

        return event_id;
    }

    void insertEvent(string timestamp, int camera_id, int eventtype_id)
    {
        data = "INSERT INTO events(timestamp, camera_id, eventtype_id) VALUES('$timestamp$', '$camera_id$', '$eventtype_id$');";
        data.replace(data.find("$timestamp$"), sizeof("$timestamp$") - 1, timestamp.c_str());
        data.replace(data.find("$camera_id$"), sizeof("$camera_id$") - 1, to_string(camera_id).c_str());
        data.replace(data.find("$eventtype_id$"), sizeof("$eventtype_id$") - 1, to_string(eventtype_id).c_str());
        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }
    void updatePathToEvent(int event_id, std::string filepath)
    {
        data = "update events set filepath = '$FILEPATH$' where id = $EVENT_ID$;";
        data.replace(data.find("$FILEPATH$"), sizeof("$FILEPATH$") - 1, filepath.c_str());
        data.replace(data.find("$EVENT_ID$"), sizeof("$EVENT_ID$") - 1, to_string(event_id).c_str());

        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    void showTable(string table)
    {
        data = "SELECT * FROM $TABLE$;";
        data.replace(data.find("$TABLE$"), sizeof("$TABLE$") - 1, table.c_str());
        rc = sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    // entweder id oder ip mitgeben
    void deleteCamera(int ID)
    {
        data = "DELETE FROM CAMERAS WHERE ID = '$ID$';";

        data.replace(data.find("$ID$"), sizeof("$ID$") - 1, to_string(ID).c_str());

        sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    std::vector<std::string> searchEntry(string table, string columns, string column, string value)
    {

        data = "SELECT $COLUMNS$ FROM $TABLE$ \r\n";
        if (column.size() > 0 && value.size() > 0)
        {
            data += "WHERE $COLUMN$ = '$VALUE$'";
        }

        data.replace(data.find("$COLUMNS$"), sizeof("$COLUMNS$") - 1, columns.c_str());
        data.replace(data.find("$TABLE$"), sizeof("$TABLE$") - 1, table.c_str());

        if (column.size() > 0 && value.size() > 0)
        {
            data.replace(data.find("$COLUMN$"), sizeof("$COLUMN$") - 1, column.c_str());
            data.replace(data.find("$VALUE$"), sizeof("$VALUE$") - 1, value.c_str());
        }

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, data.c_str(), -1, &stmt, NULL);

        std::string col_name, col_value;
        std::vector<std::string> entryValues;

        int cols = sqlite3_column_count(stmt);

        // geht jede row durch id1, id2, id..
        while (sqlite3_step(stmt) == SQLITE_ROW)
        { // geht jede spalte einer row durch
            for (int i = 0; i < cols; i++)
            {
                if (sqlite3_column_name(stmt, i))
                {
                    col_name = const_cast<const char *>(sqlite3_column_name(stmt, i));
                }
                if (sqlite3_column_text(stmt, i))
                {
                    col_value = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                    entryValues.push_back(col_value);
                }
            }
        }
        sqlite3_finalize(stmt);
        return entryValues;
    }

    void closeDB()
    {
        sqlite3_close(db);
    }
};

/*
sqlite3* dbs;
string command;
sqlite3_stmt *stmt;
string col_name;
string col_value;
int cols;

cout << "SQL Kommando eingeben:";
getline(cin,command);

if (sqlite3_prepare_v2(dbs,command.c_str(),command.size(),&stmt,0)!=SQLITE_OK)
{
   cerr << "SQL-Fehler: " << sqlite3_errmsg(dbs) << endl;
}
cols = sqlite3_column_count(stmt);

while (sqlite3_step(stmt) == SQLITE_ROW)
{
   for (int i=0;i<cols;i++)
   {
     if(sqlite3_column_name(stmt,i))
       col_name = const_cast<const char*>(sqlite3_column_name(stmt,i));
     else col_name = "LEER";

     if (sqlite3_column_text(stmt,i))
       col_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt,i));
     else col_value = "LEER";

     cout << "Inhaltstyp: " << sqlite3_column_type(stmt,i) << " ";
     cout << col_name << ": ";
     cout << col_value << endl;
    }
}
sqlite3_finalize(stmt);
*/