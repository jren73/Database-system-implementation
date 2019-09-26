#include <string>

#include "Config.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "DBFile.h"
#include "RelOp.h"
#include "Comparison.h"
#include <stdio.h>
#include <string.h>

using namespace std;


DBFile::DBFile () : fileName(""), count(0) {
}

DBFile::~DBFile () {
}

DBFile::DBFile(const DBFile& _copyMe) :
	file(_copyMe.file),	fileName(_copyMe.fileName), count(_copyMe.count) {}

DBFile& DBFile::operator=(const DBFile& _copyMe) {
	// handle self-assignment first
	if (this == &_copyMe) return *this;

	file = _copyMe.file;
	fileName = _copyMe.fileName;
	count = _copyMe.count;

	return *this;
}

int DBFile::Create (char* f_path, FileType f_type) {

	if (f_type == Heap) {
		// create heap file

		ofstream outFile;

		outFile.open(f_path, std::ofstream::binary);
		outFile.close();

	}

	fileName = f_path;

}

int DBFile::Open (char* f_path) {

	if (file.Open(1, f_path) == -1) {

		cout << "Error opening DBFile" << endl;

	}

}

void DBFile::Load (Schema& schema, char* textFile) {


	char* myFileName = new char[fileName.length() + 1];
	strcpy(myFileName, fileName.c_str());

	file.Open(0, myFileName);

	FILE* f = fopen(textFile, "r");						// Extract textFile to record
	Record r;
	int successP;
	Page p;							
	while(r.ExtractNextRecord(schema, *f) == 1)
	{
		successP = p.Append(r);							// Extract record to Page
		if (successP == 0)								// Insert old Page into File and create a new Page when the Page is empty
		{
			file.AddPage(p, file.GetLength());
			p.EmptyItOut();
			p.Append(r);

		}

	}
	
	file.AddPage(p, file.GetLength());
	/*string heapLoc = "../Binary Files/region.bin";
	char* path = new char[heapLoc.length() + 1];
	strcpy(path, heapLoc.c_str());
	file.Open(PAGE_SIZE, path);
	Page p2;
	cout << file.GetPage(p2, 1) << endl;
	*/
	/*Page p2;
	Record r1;
	cout << file.GetPage(p2, 0) << endl;
	cout << file.GetLength() << endl;
	while (p2.GetFirst(r1)) {
		r1.print(cout, schema);
		cout << endl;
	}*/
	/*
	Record myRec;
	Page p2;
	cout << file.GetPage(p2, 0) << endl;
	cout << p2.GetFirst(myRec) << endl;
	cout << myRec.GetSize() << endl;
	//p.EmptyItOut();
	*/
	

	file.Close();
}

int DBFile::Close () {

	file.Close();

}

void DBFile::MoveFirst () //issue here. Need to store the Record first some how.
{
	off_t curr_length = 0;//file.GetLength();
	Page pageTemp;						//page we are in
	file.GetPage(pageTemp, curr_length);//rusu said just do this
	//pageTemp.Mover();
}

//appends record to end of file
void DBFile::AppendRecord (Record& rec) {
	Page p;
	file.GetPage(p, file.GetLength());	//get last page
	if (p.Append(rec)) {	//append record to the last page
		std::cout << "Appended Record\n";
	}
	else {	//if failed (page has not enough space) add a new page then append the record to the new page
		Page pg;
		pg.Append(rec);
		file.AddPage(pg, file.GetLength());
		std::cout << "Added a new page and appended record\n";
	}
}

int DBFile::GetNext (Record& rec) {

	//cout << file.GetLength() << endl;
	//cout << "dbfile get next start\n";

	//if (file.GetPage(page, count) != -1) {
	if (temp.GetFirst(rec) == 0) {
		file.GetPage(temp, count);
		count++;

		if (temp.GetFirst(rec) == 0) {
			//cout << "db file get next false 1\n";
			return 0;
		}

		else {
			//cout << "db file true 1\n";
			//cout << "number of records: " << temp.numRecs << endl;
			return 1;
		}

	}
	//cout << "dbfile true 2\n";
	//cout << "number of records: " << temp.numRecs << endl;
	return 1;

}
