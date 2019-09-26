#include <iostream>
#include <string>

#include "Catalog.h"
extern "C" {
#include "QueryParser.h"
}
#include "QueryOptimizer.h"
#include "QueryCompiler.h"
#include "RelOp.h"
#include "Table.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

// these data structures hold the result of the parsing
extern struct FuncOperator* finalFunction; // the aggregate function
extern struct TableList* tables; // the list of tables in the query
extern struct AndList* predicate; // the predicate in WHERE
extern struct NameList* groupingAtts; // grouping attributes
extern struct NameList* attsToSelect; // the attributes in SELECT
extern int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query
extern char* command; // command to execute other functionalities (e.g. exit)
extern char* table; // single table name for CREATE and LOAD
extern struct AttrAndTypeList* attrAndTypes; // attributes and types to be inserted
extern char* textFile; // text file to be loaded into a table
extern char* indexName; // index name for CREATE INDEX
extern char* attr; // single attribute name for CREATE INDEX
int typeOfInput;
/* typeOfInput
 * 0 = query
 * 1 = create table
 * 2 = load into table
 * 3 = print Query execution tree
 */
 extern "C" int yyparse();
 extern "C" int yylex_destroy();

 string dbFile = "catalog.sqlite";
 Catalog catalog(dbFile);


int main(int argc, char* argv[])
{
  string _input;
  // this is the query optimizer
	// it is not invoked directly but rather passed to the query compiler
	QueryOptimizer optimizer(catalog);

	// this is the query compiler
	// it includes the catalog and the query optimizer
	QueryCompiler compiler(catalog, optimizer);

	TableSetter tableSetter(catalog);
   while(1)
  {
    /*getline (cin, _input);
    if (_input == "exit") {
      break;
    }*/

    int parse = -1;
   if (yyparse() == 0) {
       cout << "OK!" << endl;
       parse = 0;
   }
    else {
         cout << "Error: Query is not correct!" << endl;
          parse = -1;
    }

    yylex_destroy();

    if (parse != 0) return -1;


    if(parse == 0) {
			if(table != NULL && attrAndTypes != NULL) { // CREATE TABLE
				tableSetter.createTable(table, attrAndTypes);
			} else if(table != NULL && textFile != NULL) { // LOAD DATA
				tableSetter.loadData(table, textFile);
			} else if(table != NULL) { // DROP TABLE
				tableSetter.dropTable(table);
			} else if(command != NULL) { // single word commands
				if(strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
					// exit, quit
					return 0;
				} else if(strcmp(command, "schema") == 0) {
					cout << catalog << endl << endl;
				}  else if(strcmp(command, "save") == 0) {
					if(catalog.Save())
						cout << "Table Saved!" << endl << endl;
				} else {
					cout << endl << "Error: Command not found." << endl << endl;
				}
			} else {
				cout << endl << "OK!" << endl;


        QueryExecutionTree queryTree;
                compiler.Compile(tables, attsToSelect, finalFunction, predicate,
                        groupingAtts, distinctAtts, queryTree);

                cout << queryTree << endl;

                queryTree.ExecuteQuery();
			}
		}

		// reset all vars
		finalFunction = NULL; tables = NULL; predicate = NULL;
		groupingAtts = NULL; attsToSelect = NULL; distinctAtts = 0;
		command = NULL; table = NULL; attrAndTypes = NULL;
		textFile = NULL; indexName = NULL; attr = NULL;
  }


}



/*
 int main()
 {
   QueryOptimizer optimizer(catalog);
   QueryCompiler compiler(catalog, optimizer);
   while(1)
   {
     string _input;
     cout<<" Please enter a number"<<endl;
     cout<<" 0 for executing queries\n 1 for creating table\n 2 for loading data\n 3 for print query execution tree.\n";
     //cin>>typeOfInput;
     getline (cin, _input);
     if(_input == "exit") {
       break;
     }
     stringstream(_input) >> typeOfInput;
     if(typeOfInput != 0 && typeOfInput != 1 && typeOfInput != 2 && typeOfInput != 3)
     {
       cout<<"Error input!"<<endl;
       continue;
     }
     //getline (cin, _input);
     //if (_input == "exit") {
 		//	break;
 		//}

     if(typeOfInput == 1)
     {
        string dbFile1;
        int tNo;
        int aNo;
        cout << "Useage: [sqlite_file] [no_tables] [no_atts]" << endl;
        getline (cin, _input);

        istringstream ss(_input);
        int arg = 0;
        do {
        // Read a word
        string word;
        ss >> word;

        arg++;
        if(arg == 1)
          dbFile1 = word;
        else if(arg ==2)
          stringstream(word) >> tNo;
        else if(arg ==3)
          stringstream(word) >> aNo;

        // While there is more to read
        } while (ss);
        if(arg > 4)
        {
          cout << "Error input!"<<endl;
          break;
        }

        Catalog catalog1(dbFile1);
        cout << catalog1 << endl; cout.flush();

        //////////////////////////////////////////
        for (int i = 0; i < tNo; i++) {
                char tN[20]; sprintf(tN, "T_%d", i);
                string tName = tN;

                int taNo = i * aNo;
                vector<string> atts;
                vector<string> types;
                for (int j = 0; j < taNo; j++) {
                        char aN[20]; sprintf(aN, "A_%d_%d", i, j);
                        string aName = aN;
                        atts.push_back(aN);

                        string aType;
                        int at = j % 3;
                        if (0 == at) aType = "Integer";
                        else if (1 == at) aType = "Float";
                        else if (2 == at) aType = "String";
                        types.push_back(aType);
                }

                bool ret = catalog1.CreateTable(tName, atts, types);
                if (true == ret) {
                        cout << "CREATE TABLE " << tName << " OK" << endl;

                        for (int j = 0; j < taNo; j++) {
                                unsigned int dist = i * 10 + j;
                                string aN = atts[j];
                                catalog1.SetNoDistinct(tName, atts[j], dist);
                        }

                        unsigned int tuples = i * 1000;
                        catalog1.SetNoTuples(tName, tuples);

                        string path = "/home/user/DATA/" + tName + ".dat";
                        catalog1.SetDataFile(tName, path);
                }
                else {
                        cout << "CREATE TABLE " << tName << " FAIL" << endl;
                }
        }

        ////////////////////////////////
        catalog1.Save();
        cout << catalog1 << endl; cout.flush();

        ////////////////////////////////
        vector<string> tables;
        catalog1.GetTables(tables);
        for (vector<string>::iterator it = tables.begin();
                 it != tables.end(); it++) {
                cout << *it << endl;
        }
        cout << endl;

     }

     else if(typeOfInput == 2){


     }

     else if(typeOfInput == 3){

       int parse = -1;
      if (yyparse() == 0) {
          //cout << "OK!" << endl;
          parse = 0;
      }
       else {
            //cout << "Error: Query is not correct!" << endl;
             parse = -1;
       }

       yylex_destroy();

       if (parse != 0) return -1;

       QueryExecutionTree queryTree;
  			compiler.Compile(tables, attsToSelect, finalFunction, predicate,
  				groupingAtts, distinctAtts, queryTree);

  		  cout << queryTree << endl;

  			//queryTree.ExecuteQuery();

     }

     else if (typeOfInput == 0) {
      //cout<<_input<<endl;
      //YY_BUFFER_STATE buffer = yy_scan_string(const_cast<char*>(_input.c_str()));
      int parse = -1;
                if (yyparse() == 0) {
                        //cout << "OK!" << endl;
                        parse = 0;
                }
                else {
                        //cout << "Error: Query is not correct!" << endl;
                        parse = -1;
                }

                yylex_destroy();

                if (parse != 0) return -1;

 			// at this point we have the parse tree in the ParseTree data structures
 			// we are ready to invoke the query compiler with the given query
 			// the result is the execution tree built from the parse tree and optimized
 			QueryExecutionTree queryTree;
 			compiler.Compile(tables, attsToSelect, finalFunction, predicate,
 				groupingAtts, distinctAtts, queryTree);

 			//cout << queryTree << endl;

 			queryTree.ExecuteQuery();
    }
     //cout << "Enter a query and hit ctrl+D when done: " << endl;
   }
   return 0;
 }
*/
