#ifndef _TABLE_H
#define _TABLE_H

#include <regex>

#include "Catalog.h"
#include "RelOp.h"

using namespace std;


class TableSetter {
private:
	Catalog* catalog;

public:
	TableSetter(Catalog& _catalog);
	~TableSetter();

	void createTable(char* _table, AttrAndTypeList* _attrAndTypes);
	void loadData(char* _table, char* _textFile);
	void dropTable(char* _table);
};

#endif // _TABLE_SETTER_H
