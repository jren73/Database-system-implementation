# include "DataStructure.h"

using namespace std;

tableInfo::tableInfo() {
	name = "";
	path = "";
	nuTuples = 0;
	changedAttributes = false;
	add = false;
	changedTables = false;
}

tableInfo::tableInfo(string na, string pa, int tu) { 
	name = na;
	path = pa;
	nuTuples = tu;
	changedAttributes = false;
	add = false;
	changedTables = false;
};

void tableInfo::Swap(tableInfo& withMe) {
	SWAP(name, withMe.getName());
	SWAP(path, withMe.getPath());
	SWAP(nuTuples, withMe.getTuples());
	SWAP(listOfAtts, withMe.getSchema());
	SWAP(changedTables, withMe.getChangedT());
	SWAP(changedAttributes, withMe.getChangedA());
	SWAP(add, withMe.getAdd());
}

void tableInfo::CopyFrom(tableInfo& withMe) {
	this->name = withMe.getName();
	this->path = withMe.getPath();
	this->nuTuples = withMe.getTuples();
	this->listOfAtts.Clear();
	this->listOfAtts = withMe.getSchema();
	this->changedTables = withMe.getChangedT();
	this->changedAttributes = withMe.getChangedA();
	this->add = withMe.getAdd();
}

void tableInfo::setName(string na) {
	name = na;
}

void tableInfo::setPath(string pa) {
	path = pa;
}

void tableInfo::setTuples(int tu) {
	nuTuples = tu;
}

void tableInfo::setSchema(const Schema& _other) {
	listOfAtts = _other;
}

void tableInfo::setChangedA(bool changed) {
	this->changedAttributes = changed;
}

void tableInfo::setChangedT(bool changed) {
	this->changedTables = changed;
}

void tableInfo::setAdd(bool add) {
	this->add = add;
}

string& tableInfo::getName() {
	return name;
}

string& tableInfo::getPath() {
	return path;
}

int& tableInfo::getTuples() {
	return nuTuples;
}

Schema& tableInfo::getSchema() {
	return listOfAtts;
}

bool& tableInfo::getChangedA() {
	return changedAttributes;
}

bool& tableInfo::getChangedT() {
	return changedTables;
}

bool& tableInfo::getAdd() {
	return add;
}

string convertType(Type typeI) {
	string typeS;

	switch (typeI) {
	case Integer:
		typeS = "INTEGER";
		break;
	case Float:
		typeS = "FLOAT";
		break;
	case String:
		typeS = "STRING";
		break;
	default:
		typeS = "Unknown";
		break;
	}

	return typeS;
}

/*
attsInfo::attsInfo() {
	name = "";
	disVal = 0;
}

attsInfo::attsInfo(string na, Type ty, int di) { 
	name = na;
	type = ty;
	disVal = di;
};

void attsInfo::Swap(attsInfo& withMe) {
	SWAP(name, withMe.name);
	SWAP(type, withMe.type);
	SWAP(disVal, withMe.disVal);
}

void attsInfo::CopyFrom(attsInfo& withMe) {
	this->name = withMe.name;
	this->type = withMe.type;
	this->disVal = withMe.disVal;
}

void attsInfo::setName(string na) {
	name = na;
}

void attsInfo::setType(Type ty) {
	type = ty;
}

void attsInfo::setDistinct(int di) {
	disVal = di;
}

string attsInfo::getName() {
	return name;
}

Type attsInfo::getType() {
	return type;
}

int attsInfo::getDistinct() {
	return disVal;
}
*/