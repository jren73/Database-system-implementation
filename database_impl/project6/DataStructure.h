/*
 * 	Author:		Hang V Liang
 *	Purpose:	Data Structure for Database Catalog to represent the catalog table
*/
#ifndef _DataStructure_H
#define _DataStructure_H
# include "Config.h"
# include "Schema.h"
# include "Swap.h"
# include <string>
# include <iostream>
using namespace std;
// Data Structure to stores the Table Info of the Catalog
// Built with compatibility with EfficientMap and InEfficientMap in mind
struct tableInfo {
private:	
	string name;																			// metaTable info 
	string path;																			// metaTable info
	int nuTuples;																			// metaTable info
	bool add;																				// Table was added
	bool changedAttributes;																	// Attributes was changed
	bool changedTables;																		// Table was changed
	Schema listOfAtts;																		// This schema class is used to stores information regarding the metaAtrtibutes
	
public:
	tableInfo();
	tableInfo(string na, string pa, int tu);
	void Swap(tableInfo& withMe);
	void CopyFrom(tableInfo& withMe);

	// Interfacing with the object
	void setName(string na);
	void setPath(string pa);
	void setTuples(int tu);
	void setSchema(const Schema& _other);
	void setChangedT(bool changed);
	void setAdd(bool add);
	void setChangedA(bool changed);
	
	string& getName();
	string& getPath();
	int& getTuples();
	Schema& getSchema();
	bool& getChangedT();
	bool& getChangedA();
	bool& getAdd();

};

string convertType(Type typeI);

/* Made Obselete by Attributes in Schema.h
// Data Structure to stores the Atts Info of the Catalog
// Built with compatibility with EfficientMap and InEfficientMap in mind
class attsInfo { 
private:
	string name;
	Type type;
	int disVal;
	
public:
	attsInfo();
	attsInfo(string na, Type ty, int di);
	void Swap(attsInfo& withMe);
	void CopyFrom(attsInfo& withMe);

	// Interfacing with the object
	void setName(string na);
	void setType(Type ty);
	void setDistinct(int di);

	string getName();
	Type getType();
	int getDistinct();
};
*/
/*
// Catalog objects that holds informations regarding both tables of the catalog
class catalogTables {
private:
	// Might swap over to InefficientMap?
	// Map storage of the master table info
	EfficientMap<KeyString , tableInfo> tables;
	// Map storage of the attributes info
	EfficientMap<KeyString , attsInfo>  atts;

public:
	//void insert_tables(KeyString key, tableInfo data);
	//void insert_atts(KeyString key, tableInfo data);

	EfficientMap<KeyString , tableInfo>& getMapTable();
	EfficientMap<KeyString, attsInfo>& getMapAtts();
};
*/
#endif 
