#include <iostream>
#include <string>

#include "Catalog.h"
#include "QueryParser.h"
#include "QueryOptimizer.h"
#include "QueryCompiler.h"
#include "RelOp.h"

using namespace std;

int main () {

	string dbFile = "catalog.sqlite";
	Catalog catalog(dbFile);

	DBFile db;

	string filename = "tablez/supplier";

	vector <string> files;
	vector <string> tablez;
	catalog.GetTables(files);

	for (int i = 0; i< files.size(); i++)
	{
		tablez.push_back(files[i]);
		files[i]+= ".tbl";
		files[i].insert(0,"tablez/");
		cout<<files[i]<<endl;
	}

	int x = 7;

	//for (int i = 0; i < files.size(); i++)
	{
		db.Create(&filename[0],(FileType) Heap);
		Schema sch;
		/*db.Open(&filename[0]);
		Schema sch;
		Page p;
		Record r;
		int records = 0;
		db.setPage();
		while (db.GetNext(r) != 0) records++;

		cout<<"\ntotal rec "<<records;
		cout<<"\ncounting again\n";
		
		records = 0;
		db.MoveFirst();
		db.setPage();
		while (db.GetNext(r) != 0) records++;
		cout<<"\ntotal rec "<<records;*/

		catalog.GetSchema(tablez[x],sch);
		db.Load(sch, &files[x][0]);
		
		//cout<<"New CurrentLngth "<<
		db.Close();//<<endl;
	}
	



	return 0;
}

