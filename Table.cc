#ifndef _TABLE_C
#define _TABLE_C

#include "Table.h"

using namespace std;

TableRecord::TableRecord(int _number_of_tuples, string _path){
  number_of_tuples = _number_of_tuples;
  path = _path;
}

TableRecord::TableRecord(int _number_of_tuples, string _path, Schema& _s){
  number_of_tuples = _number_of_tuples;
  path = _path;
  s = _s;
}

void TableRecord::set_number_of_tuples(const int _number_of_tuples)
{
  number_of_tuples = _number_of_tuples;
}

void TableRecord::set_data_path(const string _path)
{
  path = _path;
}
void TableRecord::set_schema(const Schema& _s)
{
  s = _s;
}

int& TableRecord::get_number_of_tuples()
{
  return number_of_tuples;
}

string& TableRecord::get_data_path()
{
  return path;
}
Schema& get_Schema()
{
  return s;
}

#endif
