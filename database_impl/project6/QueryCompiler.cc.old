#include "QueryCompiler.h"
#include "QueryOptimizer.h"
#include "Schema.h"
#include "ParseTree.h"
#include "Record.h"
#include "DBFile.h"
#include "Comparison.h"
#include "Function.h"
#include <vector>
#include "EfficientMap.h"
#include "EfficientMap.cc"
#include "Keyify.h"
#include "Keyify.cc"

using namespace std;


QueryCompiler::QueryCompiler(Catalog& _catalog, QueryOptimizer& _optimizer) :
	catalog(&_catalog), optimizer(&_optimizer){
	catalog = &_catalog;
	optimizer = &_optimizer;
	_keepMe = NULL;
	joinCount = 0;
}

QueryCompiler::~QueryCompiler() {
	//to prevent memory leak
	for (int i = 0; i < SelectMap.size(); ++i) {
		delete SelectMap[i];
	}
	for (int i = 0; i < ScanMap.size(); ++i) {
		delete ScanMap[i];
	}
	//delete everything in our vector that holds everything we need to delete
	for (int i = 0; i < DeleteThis.size(); ++i) {
		delete DeleteThis[i];
	}
	if(_keepMe != NULL)
		delete _keepMe;	//delete this
}
int QueryCompiler::tableSize(TableList* _tables)
{
	int size = 0;
	TableList* iterator = _tables;
	while (iterator != NULL)
	{
		iterator = iterator->next;
		++size;
	}
	return size;
}

//goes through select/scan maps to get relational operator associated with table
RelationalOp* QueryCompiler::GetRelOp(string table) {
	//check the select map first
	for (int i = 0; i < SelectMap.size(); ++i) {
		if ( !table.compare(SelectMap[i]->getTable()) ) {	//compare the string to what we have in our map
			return SelectMap[i];
		}
	}
	for (int i = 0; i < ScanMap.size(); ++i) {
		if ( !table.compare(ScanMap[i]->getTable()) ) {	//compare the string to what we have in our map
			return ScanMap[i];
		}
	}
}

//Recursive function to traverse the optimization tree and create join operators along the way
//traverses in post order - reaches both children before parent
//returns the root of the join tree as a relational op pointer
//resulting schema can be accessed with the root's schemaOut
RelationalOp * QueryCompiler::JoinTree(OptimizationTree * node, AndList * _predicate, double _memCapacity) {
	RelationalOp* left;
	RelationalOp* right;

	if (node->leftChild != NULL) {
		left = JoinTree(node->leftChild, _predicate, _memCapacity);	//get the relational op of left child
	}
	if (node->rightChild != NULL) {
		right = JoinTree(node->rightChild, _predicate, _memCapacity);	//get the relational op of right child
	}

	//check how many tables are at this node
	//if it's only 1, then it was not a join
	//base case for queries dealing with only one table
	if (node->tables.size() < 2)
		{
		return GetRelOp(node->tables[0]);	//then search through our scan/select maps to find the relational op
	}

	//if there are 2 or more tables, then we need to join
	// gets the left/right schemas/relationalOp from the children of this node
	else {
		//make the join operator

		//get schemas
		Schema left_schema = left->GetSchema();
		Schema right_schema = right->GetSchema();

		//save left schema
		Schema left_temp = left_schema;

		//get the predicate
		CNF predicate;
		predicate.ExtractCNF(*_predicate, left_schema, right_schema);
		
		//std::cout << "predicate: " << predicate << std::endl;

		//get sum of tuples in children
		double leftTuples = 0.0;
		double rightTuples = 0.0;

		//std::cout << "left schema " << left_schema << std::endl;
		//std::cout << "right schema: " << right_schema << std::endl;

		
		//std::cout << "left tuples: \n";// << node->leftChild->noTuples << " right tuples: " << node->rightChild->noTuples << "\n";
		for (int i = 0; i < node->leftChild->tuples.size(); i++) {
			std::cout << node->leftChild->tuples[i] << std::endl;
			leftTuples += node->leftChild->tuples[i];
		}
		
		//std::cout << "right tuples: \n";// << node->noTuples << std::endl;
		for (int i = 0; i < node->rightChild->tuples.size(); i++) {
			std::cout << node->rightChild->tuples[i] << std::endl;
			rightTuples += node->rightChild->tuples[i];
		}
		

		std::cout << "left size: " << left->getSum() << std::endl;
		std::cout << "right size: " << right->getSum() << std::endl;

		std::cout << "left rel op: " << *left << std::endl;
		std::cout << "}}}}}}}}}}}}" << std::endl;

		//get resulting schema
		left_schema.Append(right_schema);	//leftschema now holds the resulting schema of join
		
		Join *j = new Join(left_temp, right_schema, left_schema, predicate, left, right, leftTuples , rightTuples, _memCapacity, joinCount);
		joinCount++;

		DeleteThis.push_back(j);	//add this to our stuff we need to delete later
		return j;	//return our relational op
	}
}

void QueryCompiler::Compile(TableList* _tables, NameList* _attsToSelect,
	FuncOperator* _finalFunction, AndList* _predicate,
	NameList* _groupingAtts, int& _distinctAtts,
	QueryExecutionTree& _queryTree, double _memCapacity) {
	
	// create a SCAN operator for each table in the query
	int size = tableSize(_tables);
	TableList *iterator=_tables;
	for (int i = 0; i < size; ++i)
	{
		Schema mySchema;
		string temp = iterator->tableName;	//get the table name
		catalog->GetSchema(temp, mySchema);	//get the schema
		DBFile myFile = DBFile();
		string pathConvert;
		catalog->GetDataFile(temp, pathConvert);
		char* path = new char[pathConvert.length() + 1];
		strcpy(path, pathConvert.c_str());
		myFile.Open(path);
		Scan *myScan = new Scan(mySchema, myFile, temp);	//make scan
		ScanMap.push_back(myScan);	//add it to scanmap
		// push-down selections: create a SELECT operator wherever necessary
		Record recTemp;
		CNF cnfTemp;
		cnfTemp.ExtractCNF(*_predicate, myScan->getSchema(), recTemp);
		Select *mySelect = new Select(myScan->getSchema(), cnfTemp, recTemp, myScan, temp);	//make select
		if ((mySelect->getCNF()).numAnds!=0)// builds CNF and Record needed. Now we have Schema, Record, and CNF. Just need RelationOp
		{
			SelectMap.push_back(mySelect);		
		}
		iterator = iterator->next;
	}

	// call the optimizer to compute the join order
	OptimizationTree *root = new OptimizationTree();
	optimizer->Optimize(_tables, _predicate, root);

	// create join operators based on the optimal order computed by the optimizer
	// j will point to root of join tree
	// call j->getSchema() to get the final schema
	RelationalOp* j = JoinTree(root, _predicate, _memCapacity);
	
	// create the remaining operators based on the query

	// connect everything in the query execution tree and return
	// Create WriteOut here
	WriteOut* writeout;

	// ASSUME: Final Join's Pointer = j
	if (_groupingAtts != 0)														// Non-empty _groupingAtts -> GroupBy
	{
		Schema _schemaIn = j->GetSchema();										// Schema from the previous relationalOp
		
		// Project input schema
		Schema _projectSchema = j->GetSchema();									// Schema to project
		int _atts_no = 0;														// Project size
		NameList* i = _groupingAtts;
		while(i != NULL){														// Find the size of _keepMe
			i = i->next;
			++_atts_no;
		}
		unsigned int distinct = 1;
		vector<int> saveMe;
		vector<int> temp;														// Vector of attributes to keep (for Project method)
		_keepMe = new int[_atts_no];											// Array of attributes to keep
		i = _groupingAtts;
		for(int a = 0; a < _atts_no; ++a)										// Fill the vector and array
		{
			string name = i->name;												// Get a Grouping Attribute name
			temp.push_back(_schemaIn.Index(name));
			distinct *= _schemaIn.GetDistincts(name);							// Get distincts from the grouping attribute
			i = i->next;
		}
		for(int b = temp.size()-1; b >= 0; --b )
		{
			_keepMe[temp.size()-1-b] = temp[b];
			saveMe.push_back(temp[b]);
		}
		_projectSchema.Project(saveMe);											// Project the schema
		
		// Create Function
		Function _compute;
		_compute.GrowFromParseTree(_finalFunction, _schemaIn);					// Insert all relevant values into Function
		
		// Create SUM Schema
		string attName = "SUM";													// Name of output schema
		string attType;															// Type of the output schema
		attType = "Float";														// attType is ALWAYS Float
		vector<string> attNames;												// vector form of Name
		attNames.push_back(attName);
		vector<string> attTypes;												// vector form of Type
		attTypes.push_back(attType);
		vector<unsigned int> distincts;											// Distincts of attributes
		distincts.push_back(distinct);											// Push the distincts
		Schema _schemaOut = Schema(attNames, attTypes, distincts);				// (SUM:(FLOAT or INTEGER) [#])
		_schemaOut.Append(_projectSchema);										// Output = Project + SUM
		
		OrderMaker _groupAtts = OrderMaker(_schemaIn, _keepMe, _atts_no);		// Insert all relevant values into OrderMaker
		
		// Create GroupBy
		GroupBy* groupby = new GroupBy(_schemaIn, _schemaOut, _groupAtts, _compute, j);		// j = Final join operator
		DeleteThis.push_back(groupby);											//make sure to delete this later

		string outFile = "output.txt";
		writeout = new WriteOut(_schemaOut, outFile, groupby);					// Insert all relevant values into WriteOut
																				// outFile is "output.txt" because we are not using it yet
	}
	else
	if (_finalFunction != 0)													// Non-empty _finalFunction -> Sum
	{
		// Create Sum here
		Schema _schemaIn = j->GetSchema();										// Schema from the previous relationalOp
		string attName = "SUM";													// Name of output schema
		string attType = "Float";												// Type of the output schema
		vector<string> attNames;
		attNames.push_back(attName);
		vector<string> attTypes;
		attTypes.push_back(attType);
		vector<unsigned int> distincts;
		distincts.push_back(1);
		Schema _schemaOut = Schema(attNames, attTypes, distincts);				// (SUM:FLOAT [1])
		Function _compute;
		_compute.GrowFromParseTree(_finalFunction, _schemaIn);					// Insert all relevant values into Function
		Sum* sum = new Sum(_schemaIn, _schemaOut, _compute, j);					// j = Final join operator
		DeleteThis.push_back(sum);	//make sure to delete this later
		string outFile = "output.txt";
		writeout = new WriteOut(_schemaOut, outFile, sum);						// Insert all relevant values into WriteOut
																				// outFile is "output.txt" because we are not using it yet
	}
	else																		// Project or Project + DuplicateRemoval
	{
		// Create Project here
		Schema _schemaIn = j->GetSchema();										// Schema from the previous relationalOp
		Schema _schemaOut = j->GetSchema();								// Output
		int _numAttsInput = _schemaIn.GetAtts().size();							// Input size
		int _numAttsOutput = 0;													// Output size
		NameList* i = _attsToSelect;
		while(i != NULL){														// Find the size of _keepMe
			i = i->next;
			++_numAttsOutput;
		}
		vector<int> saveMe;	
		vector<int> temp;													// Vector of attributes to keep (for Project method)
		_keepMe = new int [_numAttsOutput];	// Array of attributes to keep
		i = _attsToSelect;
		for(int a = 0; a < _numAttsOutput; ++a)									// Fill the vector and array
		{
			string name = i->name;
			temp.push_back(_schemaIn.Index(name));
			i = i->next;
		}
		for(int b = temp.size()-1; b >= 0; --b )
		{
			_keepMe[temp.size()-1-b] = temp[b];
			saveMe.push_back(temp[b]);
		}
		_schemaOut.Project(saveMe);	
		Project* project = new Project(_schemaIn, _schemaOut, _numAttsInput, _numAttsOutput, _keepMe, j);	// Insert results in Project
		DeleteThis.push_back(project);	//make sure to delete this later

		if (_distinctAtts != 0)													// _distinctAtts != 0 -> DuplicateRemoval
		{
			// Create DuplicateRemoval here
			int* _keepThese;
			_keepThese = new int[_numAttsOutput];
			for (int c = 0; c < _numAttsOutput; ++c)
				_keepThese[c] = c;
			OrderMaker duplComp(_schemaOut, _keepThese, _numAttsOutput);
			DuplicateRemoval* duplicateRemoval = new DuplicateRemoval(_schemaOut, project, duplComp);
			DeleteThis.push_back(duplicateRemoval);	//make sure to delete this later
			string outFile = "output.txt";
			writeout = new WriteOut(_schemaOut, outFile, duplicateRemoval);		// Insert all relevant values into WriteOut
																				// outFile is "output.txt" because we are not using it yet
		}
		else																	// Project only case
		{
			string outFile = "output.txt";
			writeout = new WriteOut(_schemaOut, outFile, project);				// Insert all relevant values into WriteOut
																				// outFile is "output.txt" because we are not using it yet
		}
	}
	// Create the QueryExecutionTree
	_queryTree = QueryExecutionTree();
	_queryTree.SetRoot(*writeout);
	DeleteThis.push_back(writeout);	//add writeout to stuff we need to delete later
}
