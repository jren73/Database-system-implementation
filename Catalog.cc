#include <iostream>
#include "sqlite3.h"
#include <cstring>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "Schema.h"
#include "Catalog.h"

using namespace std;

static int callback(void *data, int argc, char **argv, char **azColName)
{
	vector<string>* temp = (vector<string>*) data;
	for(int i=0; i<argc; i++)
	{
		temp->push_back(azColName[i]);
		temp->push_back(argv[i]);
	}
	return 0;
}

/* Catalog constructor.
 * Initialize the catalog with the persistent data stored in _fileName.
 * _fileName is a SQLite database containing data on tables and their attributes.
 * _fileName is queried through SQL.
 * Populate in-memory data structures with data from the SQLite database.
 * All the functions work with the in-memory data structures.
 */
Catalog::Catalog(string& _fileName) {
	char *ErrMsg = 0;
	char *db_name = new char [_fileName.size()+1];
	strcpy (db_name, _fileName.c_str());

	sqlite3_open(db_name, &db);

	vector<string> SavedTables;
	string sql1 = "SELECT * from ListOfTables;";
	char *sql = new char [sql1.size()+1];
	strcpy (sql, sql1.c_str());
	sqlite3_exec(db, sql, callback, (void*)&SavedTables, &ErrMsg);

	for(int i =0;i < SavedTables.size()/6; i++)
	{
		vector<string> TableData;

		//6i+3 = no of tuples
		tabList.name.push_back(SavedTables[i*6+ 1]);
		tabList.noTuples.push_back(atoi(SavedTables[i*6+ 3].c_str()));
		tabList.location.push_back(SavedTables[i*6+ 5]);


		sql1 = "SELECT * from " + SavedTables[6*i + 1];
		sql = new char [sql1.size()+1];
		strcpy (sql, sql1.c_str());

		sqlite3_exec(db, sql, callback, (void*)&TableData, &ErrMsg);

		vector<unsigned int> d;
		vector<string> _attributes, _attributeTypes;

		for (int j = 0; j < TableData.size()/6; j++)
		{
			d.push_back(atoi(TableData[j*6 + 5].c_str()));
			_attributes.push_back(TableData[j*6 + 1]);
			_attributeTypes.push_back(TableData[j*6 + 3]);
		}

		Schema s(_attributes,_attributeTypes,d);
		record[SavedTables[6*i + 1]] = s;
	}

}

Catalog::~Catalog() {
	sqlite3_close(db);
}

bool Catalog::Save() {
	char *ErrMsg = 0;
	int  rc;
	string sql1;
	char *sql = NULL;

	vector<string> tables;
	GetTables(tables);

	sql1 = 	"CREATE TABLE ListOfTables( NAME STRING,noTuplesles INTEGER, location STRING );";

	sql = new char [sql1.size()+1];
	strcpy (sql, sql1.c_str());
	sqlite3_exec(db, sql, 0, 0, &ErrMsg);

	sql1 = "DELETE FROM ListOfTables";
	sql = new char [sql1.size()+1];
	strcpy (sql, sql1.c_str());
	sqlite3_exec(db, sql, 0, 0, &ErrMsg);

	for (int i = 0; i < tables.size(); i++)
	{
		string s;
		stringstream out;
		out << tabList.noTuples[i];
		s = out.str();

		sql1 = "INSERT INTO ListOfTables VALUES( '" + tables[i] + "','" + s + "','/127.0.0.1:27015' );";

		sql = new char [sql1.size()+1];
		strcpy (sql, sql1.c_str());
		sqlite3_exec(db, sql, 0, 0, &ErrMsg);
	}

	string _table;
	vector<string> _attributes , _attributeTypes;

	for (std::map<string, Schema>::iterator it=record.begin(); it!=record.end(); ++it)
	{
		vector<Attribute> atts;
		Schema s;
		_table = it->first;
		s = it->second;
		atts = s.GetAtts();

		int Size = s.GetNumAtts();

		sql1 = 	"CREATE TABLE " + _table + "( ATTS STRING, TYPE STRING, NoDistinct INTEGER );";
		sql = new char [sql1.size()+1];
		strcpy (sql, sql1.c_str());
		sqlite3_exec(db, sql, 0, 0, &ErrMsg);

		sql1 = "DELETE FROM " + _table;
		sql = new char [sql1.size()+1];
		strcpy (sql, sql1.c_str());
		sqlite3_exec(db, sql, 0, 0, &ErrMsg);

		for (int i = 0;i < Size; i++)
		{
			sql1 = 	"INSERT INTO " + _table + " VALUES('" + atts[i].name + "' ,'";

			switch(atts[i].type) {
			case Integer:	sql1+= "INTEGER'";	break;
			case Float:	sql1+= "FLOAT'";	break;
			case String:	sql1+= "STRING'";	break;
			default:	sql1+= "UNKNOWN";	break;
			}

			string s;
			stringstream out;
			out << atts[i].noDistinct;
			s = out.str();
			sql1 = sql1 + "," + s + " );";

			sql = new char [sql1.size()+1];
			strcpy (sql, sql1.c_str());
			sqlite3_exec(db, sql, 0, 0, &ErrMsg);

		}
	}
}

bool Catalog::GetNoTuples(string& _table, unsigned int& _noTuples) {
	int i=-1;
	while(1)
	{
		i++;
		if (tabList.name[i] == _table) break;
	}
	_noTuples = tabList.noTuples[i];
	return true;
}

void Catalog::SetNoTuples(string& _table, unsigned int& _noTuples) {
}

bool Catalog::GetDataFile(string& _table, string& _location) {
	return true;
}

void Catalog::SetDataFile(string& _table, string& _location) {
}

bool Catalog::GetNoDistinct(string& _table, string& _attribute,
	unsigned int& _noDistinct) {
	return true;
}
void Catalog::SetNoDistinct(string& _table, string& _attribute,
	unsigned int& _noDistinct) {
}

void Catalog::GetTables(vector<string>& _tables) {
	for (std::map<string, Schema>::iterator it=record.begin(); it!=record.end(); ++it)
	{
		_tables.push_back(it->first);
	}
}

bool Catalog::GetAttributes(string& _table, vector<string>& _attributes) {
	return true;
}

bool Catalog::GetSchema(string& _table, Schema& _schema) {
	_schema = record[_table];
	return true;
}

bool Catalog::CreateTable(string& _table, vector<string>& _attributes,
	vector<string>& _attributeTypes) {

	vector<unsigned int> d;

	for (int i = 0; i < _attributes.size(); i++)
		d.push_back(0);

	Schema s(_attributes,_attributeTypes,d);
	record[_table] = s;
	return true;
}

bool Catalog::DropTable(string& _table) {
	char *ErrMsg = 0;
	int  rc;
	char *sql = NULL;

	string sql1;
	sql1 = 	"DROP TABLE " + _table;
	sql = new char [sql1.size()+1];
	strcpy (sql, sql1.c_str());

	rc = sqlite3_exec(db, sql, 0, 0, &ErrMsg);

	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
	}else{
		fprintf(stdout, "Table dropped successfully\n");
	}
	record.erase(_table);

	return true;
}

ostream& operator<<(ostream& _os, Catalog& _c) {
	for (std::map<string, Schema>::iterator it=_c.record.begin(); it!=_c.record.end(); ++it)
	{
		cout<<"Table Name: "<<it->first;
		Schema s = it->second;
		cout<<s<<endl;
	}
	return _os;
}
