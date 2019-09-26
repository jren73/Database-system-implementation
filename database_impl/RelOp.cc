#include <iostream>
#include "RelOp.h"

using namespace std;


ostream& operator<<(ostream& _os, RelationalOp& _op) {
	return _op.print(_os);
}

Scan::Scan(Schema& _schema, DBFile& _file, string table) {
	schema = _schema;
	s = _schema;
	file = _file;
	this->table = table;
	 cout<<"Scan Table ("<<this->table<<")"<<endl;
}

Scan::Scan()
{
	schema = Schema();
	file =  DBFile();
}

Scan::~Scan() {

}

Schema& Scan::getSchema()
{
	return schema;
}

DBFile& Scan::getFile()
{
	return file;
}

string Scan::getTable() {
	return table;
}

void Scan::Swap(Scan& withMe)
{
	SWAP(schema, withMe.getSchema());
	SWAP(file, withMe.getFile());
}

void Scan::CopyFrom(Scan& withMe)
{
	this->schema = withMe.schema;
	this->file = withMe.file;
}

ostream& Scan::print(ostream& _os) {
	return _os << "SCAN\nSchema:"<<schema;
}

Select::Select(Schema& _schema, CNF& _predicate, Record& _constants,
	RelationalOp* _producer, string table) {
	schema = _schema;
	s = _schema;
	predicate = _predicate;
	constants = _constants;
	producer = _producer;
	this->table = table;
}

Select::Select()
{}

Select::~Select() {

}

Schema& Select::getSchema()
{
	return schema;
}

CNF& Select::getCNF()
{
	return predicate;
}

Record& Select::getRecord()
{
	return constants;
}

RelationalOp* Select::getRelational()
{
	return producer;
}

string Select::getTable() {
	return table;
}

void Select::Swap(Select& withMe)
{
	SWAP(schema, withMe.schema);
	SWAP(predicate, withMe.predicate);
	SWAP(constants, withMe.constants);
	SWAP(producer, withMe.producer);
}

ostream& Select::print(ostream& _os) {
	return _os << "\nSELECT"<<"\nPredicate: "<<predicate<<"\nProducer: " << *producer << endl;
}

bool Select::GetNext(Record& _record) {

	Record record;

	while (producer->GetNext(record) == true) {

		if (predicate.Run(record, constants) == true) {	// constants = literals?
			//record.ExtractNextRecord(schema, file);	// textfile 
			//record.Swap(constants);
			//record.AppendRecords()
			_record = record;
			return true;
		}

	}
	return false;

}

Project::Project(Schema& _schemaIn, Schema& _schemaOut, int _numAttsInput,
	int _numAttsOutput, int* _keepMe, RelationalOp* _producer) {
	schemaIn = _schemaIn;
	schemaOut = _schemaOut;
	numAttsInput = _numAttsInput;
	numAttsOutput = _numAttsOutput;
	keepMe = _keepMe;
	producer = _producer; 
}

Project::~Project() {

}

ostream& Project::print(ostream& _os) {
	_os << "\nPROJECT: " << "\nSchema Out: " << schemaOut << "\n# Attributes Input: " << numAttsInput << "\n# Attributes Output: " << numAttsOutput << "\nKeep: ";
	for (int i = 0; i < numAttsOutput; i++) {
		_os << keepMe[i] << " ";
	}
	return _os << "\nProducer: " << *producer << endl;
}

bool Project::GetNext(Record& _record) {
	// Assume Project is working correctly
	// that is every private member variable is holding what it describes in header file
	while (producer->GetNext(_record)) {
		_record.Project(keepMe, numAttsOutput, numAttsInput);
		return true;
	}
	return false;
}

Join::Join(Schema& _schemaLeft, Schema& _schemaRight, Schema& _schemaOut,
	CNF& _predicate, RelationalOp* _left, RelationalOp* _right) {
	schemaLeft = _schemaLeft;
	schemaRight = _schemaRight;
	schemaOut = _schemaOut;
	s = _schemaOut;
	predicate = _predicate;
	left = _left;
	right = _right;
}

Join::~Join() {

}

Schema & Join::getSchema(){
	return schemaOut;
}

ostream& Join::print(ostream& _os) {
	return _os << "\nJOIN\nLeft Schema:\n" << schemaLeft << "\nRight Schema:\n" << schemaRight << "\nSchema Out:\n" << schemaOut << "\nPredicate:\n" << predicate << "\nLeft Operator:\n{\n" << *left << "\n}\nRight Operator:\n{\n" << *right << "\n}" << endl;
}

/*ostream& Join::print(ostream& _os) {
        return _os << "JOIN\n  Left Schema:\n" << schemaLeft << "\n  Right Schema:\n" << schemaRight << "\n  Schema Out:\n" << schemaOut << "\nPredicate:\n" << predicate << "\n  Left Operator:\n{\n" << *left << "\n}\n  Right Operator:\n{\n" << *right << "\n}" << endl;
}
*/
DuplicateRemoval::DuplicateRemoval(Schema& _schema, RelationalOp* _producer) {
	schema = _schema;
	producer = _producer;
}

DuplicateRemoval::~DuplicateRemoval() {

}

ostream& DuplicateRemoval::print(ostream& _os) {
	return _os << "\nDISTINCT \nSchema: " << schema << "\nProducer: " << *producer << endl;
}


Sum::Sum(Schema& _schemaIn, Schema& _schemaOut, Function& _compute,
	RelationalOp* _producer) {
	schemaIn = _schemaIn;
	schemaOut = _schemaOut;
	compute = _compute;
	producer = _producer;
}

Sum::~Sum() {
	
}

ostream& Sum::print(ostream& _os) {
	return _os << "\nSUM" << "\nSchemaOut: " << schemaOut << "\nFunction"<< "Producer: " << *producer << endl;//told by TA to just use "Function"
}


GroupBy::GroupBy(Schema& _schemaIn, Schema& _schemaOut, OrderMaker& _groupingAtts,
	Function& _compute,	RelationalOp* _producer) {
	schemaIn = _schemaIn;
	schemaOut = _schemaOut;
	groupingAtts.Swap(_groupingAtts);
	compute = _compute;
	producer = _producer;
}

GroupBy::~GroupBy() {

}

ostream& GroupBy::print(ostream& _os) {
	return _os << "\nGROUP BY"<< "\nSchemaOut: " << schemaOut << "\nGroupingAtts: " << groupingAtts << "\nFunction: " << "Producer:\n{\n" << *producer << "\n}" << endl;	
}


WriteOut::WriteOut(Schema& _schema, string& _outFile, RelationalOp* _producer) {
	schema = _schema;
	outFile = _outFile;
	producer = _producer;
}

WriteOut::~WriteOut() {

}

ostream& WriteOut::print(ostream& _os) {
	return _os << "OUTPUT\nSchema: " << schema << "\nProducer:\n{\n" << *producer << "\n}\n";
}


ostream& operator<<(ostream& _os, QueryExecutionTree& _op) {
	return _os << "QUERY EXECUTION TREE" << "(\n" << *_op.root << "\n)";
}
