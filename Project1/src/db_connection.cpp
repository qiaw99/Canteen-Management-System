#include <iostream>
#include "mysql.h"
#include <string>
#include <stdlib.h>
#include <iomanip>

using namespace std;

/// @brief Global variables
int qstate;
MYSQL conn;
MYSQL_RES* result;
MYSQL_ROW row;

void show_menu();

struct Result {
    bool succ;
    int value;
};

typedef Result Res;

class Canteen {
public:
    Canteen() {
        connect_to_db();
    }
    void addNewItem(char* name, int quantity);
    void addNewSale(char* name, int quantity);
    void searchById(int id);
    void searchByName(string name);
    void allItems();
    void update(char* name, int quantity);
    void soldItems();
private:
    void connect_to_db();
};

/// @brief Handle user input 
/// @param canteen The chosen canteen
void handleInput(Canteen canteen) {
    string input;

    while (input != "7") {
        show_menu();
        getline(cin, input);
        cout << "\n";

        int temp = stoi(input);

        string name;
		string quantity;
		int input_quantity;
		const char* tempChar;

        switch (temp) {
		case 1:
			cout << "Welcome to Canteen Management System!" << endl;
			break;
		case 2:
			cout << "Show all items: " << endl;
			canteen.allItems();
			break;
		case 3: 
            cout << "Show all sold items: " << endl;
			canteen.soldItems();
			break;
		case 4: 

			cout << "Please enter the name of item: "; 
            getline(cin, name);

            cout << "\nPlease enter the quantity of item: \n";
			getline(cin, quantity);

            input_quantity = stoi(quantity);
			tempChar = name.data();
			canteen.addNewItem((char*)tempChar, input_quantity);
			break;
		case 5:
			cout << "Please enter the name of item: ";
			getline(cin, name);

			cout << "\nPlease enter the quantity of item: \n";
			getline(cin, quantity);

			input_quantity = stoi(quantity);
			tempChar = name.data();
			canteen.update((char*)tempChar, input_quantity);
			break;
		case 6: 
			cout << "Please enter the name of item: ";
			getline(cin, name);

			cout << "\nPlease enter the quantity of sold item: \n";
			getline(cin, quantity);

			input_quantity = stoi(quantity);
			tempChar = name.data();
			canteen.addNewSale((char*)tempChar, input_quantity);
			break;
		default:
			break;
		}
    }
}

/// @brief Show the main menu
void show_menu() {
    cout << "Canteen Management System" << endl;
    cout << "1. New Custom" << endl;
    cout << "2. All Items" << endl;
    cout << "3. Sold Items" << endl;
    cout << "4. Add New Item" << endl;
    cout << "5. Edit Item" << endl;
    cout << "6. Delete Item" << endl;
    cout << "7. Exit" << endl;
}

/// @brief Class Canteen which includes all supported operations
void Canteen::connect_to_db() {
    if (mysql_library_init(0, NULL, NULL))
    {
        printf("could not initialize MySQL library\n");
        system("pause");
        exit(1);
    }

    // Init the connection to db
    mysql_init(&conn);

    // Connect to db
    if (mysql_real_connect(&conn, "localhost", "root", "qianli99", "canteen_management", 0, NULL, CLIENT_FOUND_ROWS)) {
        printf("connect success!\n");

        if (!mysql_set_character_set(&conn, "utf8"))
        {
            printf("Character for the connection : %s\n",
                mysql_character_set_name(&conn));
        }
    }
    else {
        printf("connect error!\n");
    }
}

/// @brief Virtualize the result in form of a table queried from db
/// @param fields A fields list containing all names of table headers
/// @param numFields number of fields
/// @return quantity of found item
string show_result(MYSQL_FIELD* fields, unsigned int numFields) {
    printf("-------------------------------------------------- \n");

    for (int i = 0; i < numFields; i++) {
        cout << "| " << setiosflags(ios::left) << setw(14) << fields[i].name << resetiosflags(ios::left);
    }
    printf(" | \n");

    string ret;

    while (row = mysql_fetch_row(result)) {

        for (int i = 0; i < numFields; i++) {
            cout << "| " << setiosflags(ios::left) << setw(14) << row[i] << resetiosflags(ios::left);
        }
        printf(" | \n");
        ret = row[numFields - 1];
    }
    printf("-------------------------------------------------- \n");
    return ret;
}

/// @brief Query function 
/// @param sql SQL query 
/// @return quantity of found item
string queryInfo(const string sql) {
    string ret;

    // != 0 --> error
    qstate = mysql_query(&conn, sql.c_str());
    if (qstate) {
        printf("error: %s\n", mysql_error(&conn));
    }
    else {
        // Get the result
        result = mysql_store_result(&conn);

        if (result == NULL) {
            printf("Error(%d) %s", mysql_errno(&conn), mysql_error(&conn));
        }
        else {
            uint64_t numRows = mysql_num_rows(result);
            cout << "Number of rows = " << numRows << endl;

            unsigned int numFields = mysql_num_fields(result);
            cout << "Number of fields = " << numFields << endl;

            MYSQL_FIELD* fields = mysql_fetch_fields(result);
            if (!fields)
                throw string("MySQL fields fetch is error!");

            ret = show_result(fields, numFields);
        }
        return ret;
    }
}

/// @brief Check whether there is enough items
/// @param name 
/// @param quantity 
/// @return Res* struct containing (bool succ, int value). 'succ' - whether can execute the operation, 'value' - available quantity of items
Res* checkQuantity(char* name, int quantity) {
    char sql[255];
    sprintf_s(sql, "select * from items where name = '%s'", name);
    string returned_quantity = queryInfo(sql);
    int quantity_of_return = stoi(returned_quantity);

    Res* res = (Res*)malloc(sizeof(Res*));

    if (quantity > quantity_of_return) {
        cout << quantity + "item(s) is not present in our canteen. Try again." << endl;
        res->succ = false;
        res->value = 0;

        return res;
    }
    else {
        res->succ = true;
        res->value = quantity_of_return;
        return res;
    }
}

/// @brief Update item and quantity in table "items"
/// @param name 
/// @param quantity 
void Canteen::update(char* name, int quantity) {
    char sql[255];
    memset(sql, '\0', sizeof(sql));
    sprintf_s(sql, "update items set quantity = %d where name = '%s'", quantity, name);
    if (mysql_query(&conn, sql)) {
        cout << "Error!" << endl;
    }
    cout << "Updated " << mysql_affected_rows(&conn) << " rows." << endl;
    queryInfo("select * from items");
}

/// @brief Add new item into table "items"
/// @param name 
/// @param quantity 
void Canteen::addNewItem(char* name, int quantity) {
    char sql[255];
    sprintf_s(sql, "select * from items where name='%s'", name);

    int temp = mysql_query(&conn, sql);
    if (temp || !(result = mysql_store_result(&conn))) {
        printf("error[%d]: %s", mysql_errno(&conn), mysql_error((&conn)));
    } else {
        if (mysql_num_rows(result)) {
            printf("Record exists already!");
            Canteen::update(name, quantity);
        } else {
            memset(sql, '\0', sizeof(sql));
            sprintf_s(sql, "insert into items (name, quantity) values ('%s', %d)", name, quantity);

            if (mysql_query(&conn, sql)) {
                cout << "Error!" << endl;
            }
            cout << "Inserted " << mysql_affected_rows(&conn) << " rows." << endl;
            queryInfo("select * from items");
        }
    }

}

/// @brief Add new sold items into table "sold'
/// @param name 
/// @param quantity 
void Canteen::addNewSale(char* name, int quantity) {
    char sql[255];
    sprintf_s(sql, "insert into sold (name, quantity) values ('%s', %d)", name, quantity);
    Res* res = checkQuantity(name, quantity);
    if (res -> succ) {
        if (mysql_query(&conn, sql)) {
            cout << "Error!" << endl;
        }
        cout << "Inserted " << mysql_affected_rows(&conn) << " rows." << endl;
        queryInfo("select * from sold");

        int aval = res->value;

        update(name, aval - quantity);
    }
    
}

/// @brief Search item by ID
/// @param id 
void Canteen::searchById(int id) {
    char sql[255];
    sprintf_s(sql, "select * from items where id = %d", id);
    queryInfo(sql);
}

/// @brief Search item by name
/// @param name 
void Canteen::searchByName(string name) {
    queryInfo("select * from items where name = '" + name + "'");
}

/// @brief Show items from table "items"
void Canteen::allItems() {
    queryInfo("select * from items");
}

/// @brief Show sold items from table "sold"
void Canteen::soldItems() {
    queryInfo("select * from sold");
}

/// @brief Drive code
/// @return 
int main() {
    Canteen canteen;
    handleInput(canteen);
    mysql_free_result(result);
    mysql_close(&conn);
    mysql_library_end();
}
