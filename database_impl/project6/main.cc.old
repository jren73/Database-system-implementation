#include <iostream>
#include <string>

#include "Catalog.h"
extern "C" {
#include "QueryParser.h"
}
#include "QueryOptimizer.h"
#include "QueryCompiler.h"
#include "RelOp.h"
#include <fstream>
#include <vector>
#include "BPlusTree.h"
#include "BPlusTree.cc"

using namespace std;


// these data structures hold the result of the parsing
extern struct FuncOperator* finalFunction; // the aggregate function
extern struct TableList* tables; // the list of tables in the query
extern struct AndList* predicate; // the predicate in WHERE
extern struct NameList* groupingAtts; // grouping attributes
extern struct NameList* attsToSelect; // the attributes in SELECT
extern int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query
// Hang's Stuff
/* typeOfInput
 * 0 = query 
 * 1 = create table
 * 2 = load into table
 * 3 = create index
 */
extern int typeOfInput;	   
extern struct GenericName* genName;	// Name for non-query scenerio
extern struct AttsList*	attsExpression;		// Atts Expression

typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern "C" YY_BUFFER_STATE yy_scan_string(char*);
extern "C" void yy_delete_buffer(YY_BUFFER_STATE);

extern "C" int yyparse();
extern "C" int yylex_destroy();

string dbFile = "catalog.sqlite";
Catalog catalog(dbFile);


string parseType(string input) {

	string integer1 = "Integer";
	string float1 = "Float";
	string string1 = "String";

	if (input[0] == 'i') {
		return integer1;
	}

	else if (input[0] == 'd') {
		return float1;
	}

	return string1;

}

void loadSingleData(string tabName, string heapLoc, string textLoc) {

	FileType file_type;
	file_type = Heap;


	DBFile myDBFile = DBFile();


	// Create 
	char* path = new char[heapLoc.length() + 1];
	strcpy(path, heapLoc.c_str());
	myDBFile.Create(path, file_type);

	// Load 
	Schema schema;
	catalog.GetSchema(tabName, schema);

	path = new char[textLoc.length() + 1];
	strcpy(path, textLoc.c_str());
	myDBFile.Load(schema, path);

	catalog.SetDataFile(tabName, heapLoc);


}


void loadData() {

	FileType file_type;
	file_type = Heap;


	DBFile myDBFile;
	string tabName;
	string heapLoc;
	string textLoc;


	vector<string> info;

	info.push_back("customer");
	info.push_back("../Binary Files/customer.bin");
	info.push_back("../TxtFiles/customer.txt");
	info.push_back("lineitem");
	info.push_back("../Binary Files/lineitem.bin");
	info.push_back("../TxtFiles/lineitem.txt");
	info.push_back("nation");
	info.push_back("../Binary Files/nation.bin");
	info.push_back("../TxtFiles/nation.txt");
	info.push_back("orders");
	info.push_back("../Binary Files/orders.bin");
	info.push_back("../TxtFiles/orders.txt");
	info.push_back("part");
	info.push_back("../Binary Files/part.bin");
	info.push_back("../TxtFiles/part.txt");
	info.push_back("partsupp");
	info.push_back("../Binary Files/partsupp.bin");
	info.push_back("../TxtFiles/partsupp.txt");
	info.push_back("region");
	info.push_back("../Binary Files/region.bin");
	info.push_back("../TxtFiles/region.txt");
	info.push_back("supplier");
	info.push_back("../Binary Files/supplier.bin");
	info.push_back("../TxtFiles/supplier.txt");



	for (int i = 0; i < info.size(); i+=3) {

	myDBFile = DBFile();

	// Create 
	char* path = new char[info[i+1].length() + 1];
	strcpy(path,info[i+1].c_str());
	myDBFile.Create(path, file_type);

	// Load 
	Schema schema;
	catalog.GetSchema(info[i], schema);

	path = new char[info[i+2].length() + 1];
	strcpy(path, info[i+2].c_str());
	myDBFile.Load(schema, path);

	catalog.SetDataFile(info[i], info[i+1]);

	}

	exit(0);



}

int createIndex(File &_bPlusFile, string &_attTemp, string &_indexTemp, string &_tableName)
{
	//create B+ Tree
	Page pageTemp; // temporary page value to not damage real page
	int pageNumber = 0; // which page
	int keyLeaf = (PAGE_SIZE - sizeof(leafNode*)) / (3.0f * sizeof(int));
	int keyInternal = (PAGE_SIZE - sizeof(leafNode*)) / ((float)(sizeof(int) + sizeof(leafNode*)));
	if (keyLeaf != keyInternal) {
		cout << "keyLeaf and keyInternal not the same\n";
		cout << "keyLeaf: " << keyLeaf << "\nkeyInternal: " << keyInternal << endl;
		return -1;
	}
	BPlusTree bTemp(keyLeaf); // needs to be modified later. value inserted will be page size
	Schema schemaTemp; // building schema to correctly insert into b+ tree
	catalog.GetSchema(_tableName, schemaTemp);
	int index = schemaTemp.Index(_attTemp); //which attribute from schema
	while (_bPlusFile.GetPage(pageTemp, pageNumber)!=-1)
	{
		int RecCounter = 0; // which record (index)
		Record recTemp; // record temp to grab from page
		char* valTemp; // value in record (convert to int)
		while (pageTemp.GetFirst(recTemp))
		{
			valTemp = recTemp.GetColumn(index);
			int* intTemp = (int*)valTemp;
			bTemp.Insert(*intTemp, pageNumber, RecCounter);
			//cout << "IntTemp: " << *intTemp << endl;
			//cout << "RecCounter: " << RecCounter << endl;
			//cout << "PageCounter: " << pageNumber << endl;
			++RecCounter;
		}
		++pageNumber;
	}
	// now export
	//cout << "export" << endl;
	vector<string> attNames;
	vector<string> attTypes;
	vector<unsigned int> distincts;
	attNames.push_back("index_key");
	attNames.push_back("child_page_number");
	attTypes.push_back("Integer");
	attTypes.push_back("Integer");
	distincts.push_back(1); // HANG SAYS SO
	distincts.push_back(1); // HANG SAYS SO
	Schema internalNode, leafNode;
	internalNode = Schema(attNames, attTypes, distincts);
	attNames.clear();
	attNames.push_back("index_key");
	attNames.push_back("data_page_number");
	attNames.push_back("record_number");
	attTypes.push_back("Integer");
	distincts.push_back(1);
	leafNode = Schema(attNames, attTypes, distincts);
	IndexFile indexFilePtr;
	string yunTemp = _indexTemp + ".bin";
	char* fileName = new char[yunTemp.length() + 1];
	strcpy(fileName, yunTemp.c_str());
	indexFilePtr.CreateIndex(fileName, Index);

	indexFilePtr.OpenIndex(fileName);
	bTemp.writeToDisk(&indexFilePtr,internalNode,leafNode);

	// Error doesn't occur here so far (not sure about logic)

	// Testing the .bin
	/*indexFilePtr.OpenIndex(fileName);
	Record rtemp;
	int nodeType;
	cout << "Let's try accessing the file" << endl;
	while (indexFilePtr.GetNextIndex(rtemp, nodeType))
	{
		if(nodeType == 0){
			rtemp.print(cout, internalNode); cout<<"hi" << endl;
		}
		else {
			rtemp.print(cout, leafNode); cout<<"bye" << endl;
		}
	}
	indexFilePtr.CloseIndex();
	cout << "Reading is successful" << endl;*/
	return 1;
}

int main()
{
	string _stmt;

	while (true) {

		int numPages = 100;

		//cout << "Enter number of pages:\n";
		//cin >> numPages;

		double memCapacity = numPages * PAGE_SIZE;


		string dbFile = "catalog.sqlite";
		Catalog catalog(dbFile);

		// this is the query optimizer
		// it is not invoked directly but rather passed to the query compiler
		QueryOptimizer optimizer(catalog);
		// this is the query compiler
		// it includes the catalog and the query optimizer
		QueryCompiler compiler(catalog, optimizer);

		// the query parser is accessed directly through yyparse
		// this populates the extern data structures

		getline(cin, _stmt);

		if (_stmt == "exit") {
			break;
		}
		
		YY_BUFFER_STATE buffer = yy_scan_string(const_cast<char*>(_stmt.c_str()));

		int parse = -1;
		if (yyparse() == 0) {
			cout << "OK!" << endl;
			parse = 0;
		}
		else {
			//cout << "Error: Query is not correct!" << endl;
			parse = -1;
		}

		yy_delete_buffer(buffer);
		yylex_destroy();
		//cout<<"Yun is bad and this part is his fault: " << typeOfInput << endl;

		// Create
		if (typeOfInput == 1) {

			string myNewTable;
			vector<string> myAttNames;
			vector<string> myAttTypes;

			if (genName != NULL) {

				//cout << genName->code << endl;
				//cout << genName->name << endl;
				//cout << endl;

				myNewTable = genName->name;

				genName = genName->next;

			}

			//cout << endl;

			while (attsExpression != NULL) {

				//cout << attsExpression->name << endl;
				//cout << attsExpression->type << endl;
				//cout << endl;

				myAttNames.push_back(attsExpression->name);

				myAttTypes.push_back(parseType(attsExpression->type));

				attsExpression = attsExpression->next;

			}

			catalog.CreateTable(myNewTable, myAttNames, myAttTypes);
			catalog.Save();
			//exit(0);

		}


		// Load
		else if (typeOfInput == 2) {

			cout << "Holding this L" << endl;

			string myLoadTable;
			string myLoadBin = "../Binary Files/";
			string myLoadText = "../TxtFiles/";

			if (genName != NULL) {

				myLoadTable = genName->name;

				myLoadBin += myLoadTable;
				myLoadBin += ".bin";


				genName = genName->next;

				myLoadText += genName->name;

			}

			//cout << myLoadTable << endl << myLoadBin << endl << myLoadText << endl;

			loadSingleData(myLoadTable, myLoadBin, myLoadText);
			//exit(0);

		}

		// Index
		else if (typeOfInput == 3)
		{
			//cout << "We are in B+ Tree, Hang is Bad and nothing works" << endl;
			File bPlusFile;
			File yunFile; //this is a file for IndexFile. It is named yun cuz he didn't set it up right.
			string tableTemp;
			string indexTemp;
			string attTemp;
			GenericName * temp = genName;
			while (temp != NULL)
			{
				if (temp->code == 0)
				{
					tableTemp = temp->name;
				}
				else if (temp->code == 1)
				{
					indexTemp = temp->name;
				}
				else if (temp->code == 3)
				{
					attTemp = temp->name;
				}
				temp = temp->next;
			}
			string yunPath;
			catalog.GetDataFile(tableTemp, yunPath);
			char* yunPls = new char[yunPath.length() + 1];
			strcpy(yunPls, yunPath.c_str());
			bPlusFile.Open(1, yunPls);
			int success = createIndex(bPlusFile, attTemp, indexTemp, tableTemp);
			delete yunPls;
			if (success == -1)
				cout << "Hang screwed up math" << endl;
			cout << "Anything after this part isn't Yun/Jacob's fault" << endl;
			//exit(0);
		}


		else if (typeOfInput == 0) {

			// at this point we have the parse tree in the ParseTree data structures
			// we are ready to invoke the query compiler with the given query
			// the result is the execution tree built from the parse tree and optimized
			QueryExecutionTree queryTree;
			compiler.Compile(tables, attsToSelect, finalFunction, predicate,
				groupingAtts, distinctAtts, queryTree, memCapacity);

			cout << queryTree << endl;

			queryTree.ExecuteQuery();


			/*OptimizationTree ptr;

			optimizer.getUniqueOrder(tables, predicate);
			optimizer.Optimize(tables, predicate, &ptr);
			*/

			//exit(0);

		}

	}
	return 0;
}

/*
DBFile myDBFile;

myDBFile.Open("../Binary Files/nation.bin");

Record myRec;
myDBFile.GetNext(myRec);

Schema schema;

string tName = "nation";

catalog.GetSchema(tName, schema);

cout << myRec.GetSize() << endl;

myRec.print(cout, schema);
*/