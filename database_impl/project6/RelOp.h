#ifndef _REL_OP_H
#define _REL_OP_H

#include <iostream>

#include "Schema.h"
#include "Record.h"
#include "DBFile.h"
#include "Function.h"
#include "Comparison.h"
#include "EfficientMap.h"
#include "Keyify.h"

using namespace std;


class RelationalOp {
protected:
	// the number of pages that can be used by the operator in execution
	int noPages;
	int counter;	// Delete Me
	Schema s;	//defined as the schema out

public:
	// empty constructor & destructor
	RelationalOp() : noPages(-1) {}
	virtual ~RelationalOp() {}

	// set the number of pages the operator can use
	void SetNoPages(int _noPages) {noPages = _noPages;}

	Schema& GetSchema() { return s; }

	// every operator has to implement this method
	virtual bool GetNext(Record& _record) = 0;

	/* Virtual function for polymorphic printing using operator<<.
	 * Each operator has to implement its specific version of print.
	 */
    virtual ostream& print(ostream& _os) = 0;

    /* Overload operator<< for printing.
     */
    friend ostream& operator<<(ostream& _os, RelationalOp& _op);
};

class ScanIndex : public RelationalOp {
private:
	// schema of records in operator
	Schema schema, toSendSch;

	// selection predicate in conjunctive normal form
	CNF predicate;
	// constant values for attributes in predicate
	Record constants;

	string table, attribute, key;
	DBFile index, header;
	

	bool doOnce;
	Schema LeafSch, InternalSch;
	vector <Record> List;
	int listCount;


	/*SCHEMA TO SEARCH INDEX FILE
	SCHEMA PAGE INT
	SCHEMA RECORD INT
	
	
	*/

public:
	ScanIndex(Schema& _schema, Schema& _toSendSch, DBFile& _index, DBFile& _header,
	CNF& _predicate, Record& _constants, string& _table, string& _attribute );

	virtual ~ScanIndex();

	virtual void returnSchema(Schema & _schema) { _schema = schema; }

	virtual bool GetNext(Record& _record);

	virtual ostream& print(ostream& _os);
};

class Scan : public RelationalOp {
private:
	// schema of records in operator
	Schema schema;

	// physical file where data to be scanned are stored
	DBFile file;

	string table;

public:
	Scan(Schema& _schema, DBFile& _file, string table);
	Scan();
	virtual ~Scan();

	void Swap(Scan& withMe);
	void CopyFrom(Scan& withMe);

	virtual bool GetNext(Record& _record);

	Schema& getSchema();
	DBFile& getFile();
	string getTable();

	virtual ostream& print(ostream& _os);
};

class Select : public RelationalOp {
private:
	// schema of records in operator
	Schema schema;

	// selection predicate in conjunctive normal form
	CNF predicate;
	// constant values for attributes in predicate
	Record constants;

	// operator generating data
	RelationalOp* producer;

	string table;

public:
	Select(Schema& _schema, CNF& _predicate, Record& _constants,
		RelationalOp* _producer, string table);
	virtual ~Select();

	Select();

	void Swap(Select& withMe);
	void CopyFrom(Select& withMe);

	Schema& getSchema();
	CNF& getCNF();
	Record& getRecord();
	RelationalOp* getRelational();
	string getTable();

	virtual bool GetNext(Record& _record);

	virtual ostream& print(ostream& _os);
};

class Project : public RelationalOp {
private:
	// schema of records input to operator
	Schema schemaIn;
	// schema of records output by operator
	Schema schemaOut;

	// number of attributes in input records
	int numAttsInput;
	// number of attributes in output records
	int numAttsOutput;
	// index of records from input to keep in output
	// size given by numAttsOutput
	int* keepMe;

	// operator generating data
	RelationalOp* producer;

public:
	Project(Schema& _schemaIn, Schema& _schemaOut, int _numAttsInput,
		int _numAttsOutput, int* _keepMe, RelationalOp* _producer);
	virtual ~Project();

	virtual bool GetNext(Record& _record);

	virtual ostream& print(ostream& _os);
};

class Join : public RelationalOp {
private:
	// schema of records in left operand
	Schema schemaLeft;
	// schema of records in right operand
	Schema schemaRight;
	// schema of records output by operator
	Schema schemaOut;


	// Data Structure Build
	vector<Record*> myDS;
	vector<Record*> appendRecords;
	int appendIndex;
	bool buildCheck;

	// for GetNext() with hash map
  EfficientMap<KeyInt, Record> map;

	// selection predicate in conjunctive normal form
	CNF predicate;

	// operators generating data
	RelationalOp* left;
	RelationalOp* right;

	OrderMaker om;

	void Nest_Loop_Join(Record& _record);
	void Hash_Join(Record& _record);
	void Symmetric_Hash_Join(Record& _record);

public:
	Join(Schema& _schemaLeft, Schema& _schemaRight, Schema& _schemaOut,
		CNF& _predicate, RelationalOp* _left, RelationalOp* _right);
	virtual ~Join();

	virtual bool GetNext(Record& _record);

	Schema& getSchema();	//function to get schemaOut

	virtual ostream& print(ostream& _os);
};

class DuplicateRemoval : public RelationalOp {
private:
	// schema of records in operator
	Schema schema;

	// operator generating data
	RelationalOp* producer;

	//struct sComp(Schema _schema);

	bool check; //check if first

	EfficientMap <Record, KeyInt> duplTemp;

	//vector <Record*> duplTemp;//store non-duplicates

	OrderMaker duplComp; //	orderMaker for duplicateRemoval


public:
	DuplicateRemoval(Schema& _schema, RelationalOp* _producer, OrderMaker &_duplComp);
	virtual ~DuplicateRemoval();

	virtual bool GetNext(Record& _record);

	virtual ostream& print(ostream& _os);
};

class Sum : public RelationalOp {
private:
	// schema of records input to operator
	Schema schemaIn;
	// schema of records output by operator
	Schema schemaOut;

	// function to compute
	Function compute;

	// operator generating data
	RelationalOp* producer;

	bool first;

public:
	Sum(Schema& _schemaIn, Schema& _schemaOut, Function& _compute,
		RelationalOp* _producer);
	virtual ~Sum();

	virtual bool GetNext(Record& _record);

	virtual ostream& print(ostream& _os);
};

class GroupBy : public RelationalOp {
private:
	// schema of records input to operator
	Schema schemaIn;
	// schema of records output by operator
	Schema schemaOut;

	// grouping attributes
	OrderMaker groupingAtts;
	// function to compute
	Function compute;

	// operator generating data
	RelationalOp* producer;

	// for GetNext()
	EfficientMap<Record, KeyDouble> map;
	bool first;
	int counter;

public:
	GroupBy(Schema& _schemaIn, Schema& _schemaOut, OrderMaker& _groupingAtts,
		Function& _compute,	RelationalOp* _producer);
	virtual ~GroupBy();

	virtual bool GetNext(Record& _record);

	virtual ostream& print(ostream& _os);
};

class WriteOut : public RelationalOp {
private:
	// schema of records in operator
	Schema schema;

	// output file where to write the result records
	string outFile;

	// operator generating data
	RelationalOp* producer;

public:
	WriteOut(Schema& _schema, string& _outFile, RelationalOp* _producer);
	virtual ~WriteOut();

	virtual bool GetNext(Record& _record);

	virtual ostream& print(ostream& _os);
};


class QueryExecutionTree {
private:
	RelationalOp* root;

public:
	QueryExecutionTree() {}
	virtual ~QueryExecutionTree() {}

	void ExecuteQuery();
	void SetRoot(RelationalOp& _root) {root = &_root;}

    friend ostream& operator<<(ostream& _os, QueryExecutionTree& _op);
};

#endif //_REL_OP_H
