#ifndef _TABLE_H
#define _TABLE_H

#include <iostream>
#include "Schema.h"

using namespace std;

class TableRecord{
private:
  int number_of_tuples = 0;
  string path="";
  Schema s;

public:
  TableRecord(){}
  TableRecord(int _number_of_tuples, string _path);
  TableRecord(int _number_of_tuples, string _path, Schema& _s);
  void set_number_of_tuples(const int _number_of_tuples);
	void set_data_path(const string _path);
	void set_schema(const Schema& _s);
  int& get_number_of_tuples();
	string& get_data_path();
	Schema& get_Schema();
  ~TableRecord(){}
};

#endif
