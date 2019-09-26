#include "Table.h"

using namespace std;
using namespace regex_constants;

TableSetter::TableSetter(Catalog& _catalog) : catalog(&_catalog) { }

TableSetter::~TableSetter() { }

void TableSetter::createTable(char* _table, AttrAndTypeList* _attrAndTypes) {
	cout << "Create table... " << flush;
	smatch m;
	regex type_int("INT|INTEGER", ECMAScript | icase);
	regex type_float("FLOAT", ECMAScript | icase);
	regex type_string("STR|STRING", ECMAScript | icase);

	string table = string(_table);
	vector<string> attrs, attrTypes;
	while(_attrAndTypes != NULL) {
		// name
		string attr = string(_attrAndTypes->name);
		attrs.push_back(_attrAndTypes->name);

		// type
		string attrType = string(_attrAndTypes->type);
		if(regex_match(attrType, m, type_int)) {
			attrTypes.push_back("INTEGER");
		} else if(regex_match(attrType, m, type_float)) {
			attrTypes.push_back("FLOAT");
		} else if(regex_match(attrType, m, type_string)) {
			attrTypes.push_back("STRING");
		}	else {
			cerr << attrType << " is not supported type." << endl << endl;
			return;
		}	

		// advance
		_attrAndTypes = _attrAndTypes->next;
	}

	// reverse the vector; _attrAndTypes has reverse order of attributes
	reverse(attrs.begin(), attrs.end());
	reverse(attrTypes.begin(), attrTypes.end());

	if(catalog->CreateTable(table, attrs, attrTypes)) {
		cout << "OK!" << endl << endl;
	} else {
		// cerr << "ERROR: Failed to create table." << endl << endl;
	}
}

void TableSetter::loadData(char* _table, char* _textFile) {
	cout << "Load data... from "<<_textFile<<"to table "<<_table<<endl << flush;
	string table(_table); // table name

	// get heap path from catalog
	string heapPath = _textFile; 
	
	//if(!catalog->GetDataFile(table, heapPath)) { return; }
	
	// get schema from catalog
	Schema schema;
	if(!catalog->GetSchema(table, schema)) { return; }

	// open DBFile
	DBFile dbFile;
	char* heapPathC = new char[heapPath.length()+1];
	strcpy(heapPathC, heapPath.c_str());
	//if(dbFile.Open(_textFile) == -1) { return; }

	// load text data into heap file
	dbFile.Load(schema, _textFile);

	// close DBFile
	if(dbFile.Close() == -1) { return; }

	//cout << "OK!" << endl << endl;
}

void TableSetter::dropTable(char* _table) {
	cout << "Drop table... " << flush;
	string table(_table); // table name

	if(catalog->DropTable(table)) {
		//cout << "OK!" << endl << endl;
	}

	_table = NULL;
}
