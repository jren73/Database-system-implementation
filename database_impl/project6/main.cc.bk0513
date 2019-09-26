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

using namespace std;


// these data structures hold the result of the parsing
extern struct FuncOperator* finalFunction; // the aggregate function
extern struct TableList* tables; // the list of tables in the query
extern struct AndList* predicate; // the predicate in WHERE
extern struct NameList* groupingAtts; // grouping attributes
extern struct NameList* attsToSelect; // the attributes in SELECT
extern int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query

extern "C" int yyparse();
extern "C" int yylex_destroy();

string dbFile = "catalog.sqlite";
Catalog catalog(dbFile);
/*
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
*/
int main()
{

		//cout << "Enter a query and hit ctrl+D when done: " << endl;
		// this is the catalog
		string dbFile = "catalog.sqlite";
		//Catalog catalog(dbFile);

		// this is the query optimizer
		// it is not invoked directly but rather passed to the query compiler
		QueryOptimizer optimizer(catalog);
		// this is the query compiler
		// it includes the catalog and the query optimizer
		QueryCompiler compiler(catalog, optimizer);


		// the query parser is accessed directly through yyparse
		// this populates the extern data structures
		int parse = -1;
		if (yyparse() == 0) {
			cout << "OK!" << endl;
			parse = 0;
		}
		else {
			//cout << "Error: Query is not correct!" << endl;
			parse = -1;
		}

		yylex_destroy();

		if (parse != 0) return -1;
		/*
		char result;

		cout << "Create heap files (Make sure the Binary Files folder is empty)? y for yes, n for no: " << endl;
		cin >> result;

		if (result == 'y') {
			loadData();
		}*/
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

		// at this point we have the parse tree in the ParseTree data structures
		// we are ready to invoke the query compiler with the given query
		// the result is the execution tree built from the parse tree and optimized
		QueryExecutionTree queryTree;
		compiler.Compile(tables, attsToSelect, finalFunction, predicate,
			groupingAtts, distinctAtts, queryTree);

		cout << queryTree << endl;

		queryTree.ExecuteQuery();
/*
		OptimizationTree ptr;
		
		optimizer.getUniqueOrder(tables, predicate);
		optimizer.Optimize(tables, predicate, &ptr);
		*/

	return 0;

}
