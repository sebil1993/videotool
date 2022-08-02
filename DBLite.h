#include <iostream>
#include <string>
#include <sqlite3.h>

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

    void createTable()
    {

        // Erstes Mal Starten
        data = "CREATE TABLE CAMERAS (ID INT PRIMARY KEY NOT NULL, IPADDRESS VARCHAR(255) NOT NULL, USERNAME VARCHAR(255) DEFAULT '', PASSWORD VARCHAR(255) DEFAULT '', STREAMURI VARCHAR(255) DEFAULT '', SNAPSHOTURI VARCHAR(255) DEFAULT '');";
        rc = sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    void insertData(string ID, string IPAddress)
    {
        data = "INSERT INTO CAMERAS ('ID', 'IPADDRESS') VALUES ('$ID$','$IPAddress$');";
        data.replace(data.find("$ID$"), sizeof("$ID$") - 1, ID.c_str());
        data.replace(data.find("$IPAddress$"), sizeof("$IPAddress$") - 1, IPAddress.c_str());

        sqlite3_exec(db,data.c_str(),callback,0,&zErrMsg);
        // cout << data << endl;
    }
    void insertData(string ID, string IPAddress, string username, string password)
    {
        data = "INSERT INTO CAMERAS ('ID', 'IPADDRESS', 'USERNAME', 'PASSWORD') VALUES ('$ID$','$IPAddress$','$username$','$password$');";
        data.replace(data.find("$ID$"), sizeof("$ID$") - 1, ID.c_str());
        data.replace(data.find("$IPAddress$"), sizeof("$IPAddress$") - 1, IPAddress.c_str());
        data.replace(data.find("$username$"), sizeof("$username$") - 1, username.c_str());
        data.replace(data.find("$password$"), sizeof("$password$") - 1, password.c_str());

        sqlite3_exec(db,data.c_str(),callback,0,&zErrMsg);
        // cout << ID << " " << IPAddress << " " << username << " " << password << endl;
    }
    void insertData(string ID, string IPAddress, string username, string password, string streamURI)
    {
        data = "INSERT INTO CAMERAS ('ID', 'IPADDRESS', 'USERNAME', 'PASSWORD', 'STREAMURI') VALUES ('$ID$','$IPAddress$','$username$','$password$','$streamURI$');";
        data.replace(data.find("$ID$"), sizeof("$ID$") - 1, ID.c_str());
        data.replace(data.find("$IPAddress$"), sizeof("$IPAddress$") - 1, IPAddress.c_str());
        data.replace(data.find("$username$"), sizeof("$username$") - 1, username.c_str());
        data.replace(data.find("$password$"), sizeof("$password$") - 1, password.c_str());
        data.replace(data.find("$streamURI$"), sizeof("$streamURI$") - 1, streamURI.c_str());

        sqlite3_exec(db,data.c_str(),callback,0,&zErrMsg);
        // cout << ID << " " << IPAddress << " " << username << " " << password << " " << streamURI << endl;
    }
    void insertData(string ID, string IPAddress, string username, string password, string streamURI, string snapshotURI)
    {
        data = "INSERT INTO CAMERAS ('ID', 'IPADDRESS', 'USERNAME', 'PASSWORD', 'STREAMURI', 'SNAPSHOTURI') VALUES ('$ID$','$IPAddress$','$username$','$password$','$streamURI$','$snapshotURI$');";
        data.replace(data.find("$ID$"), sizeof("$ID$") - 1, ID.c_str());
        data.replace(data.find("$IPAddress$"), sizeof("$IPAddress$") - 1, IPAddress.c_str());
        data.replace(data.find("$username$"), sizeof("$username$") - 1, username.c_str());
        data.replace(data.find("$password$"), sizeof("$password$") - 1, password.c_str());
        data.replace(data.find("$streamURI$"), sizeof("$streamURI$") - 1, streamURI.c_str());
        data.replace(data.find("$snapshotURI$"), sizeof("$snapshotURI$") - 1, snapshotURI.c_str());

        sqlite3_exec(db,data.c_str(),callback,0,&zErrMsg);
        // cout << ID << " " << IPAddress << " " << username << " " << password << " " << streamURI << " " << snapshotURI << endl;
    }

    void showTable()
    {
        data = "SELECT * FROM 'CAMERAS';";
        rc = sqlite3_exec(db, data.c_str(), callback, 0, &zErrMsg);
    }

    void deleteCamera(string ID)
    {
        data = "DELETE FROM 'CAMERAS' WHERE ID = '$ID$';";
        data.replace(data.find("$ID$"), sizeof("$ID$") - 1, ID.c_str());
        
        sqlite3_exec(db,data.c_str(),callback,0,&zErrMsg);
    }

    void closeDB()
    {
        sqlite3_close(db);
    }
};