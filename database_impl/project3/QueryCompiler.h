#ifndef _QUERY_COMPILER_H
#define _QUERY_COMPILER_H

/* Take as input the query tokens produced by the query parser and generate
 * the query execution tree. This requires instantiating relational operators
 * with the correct parameters, based on the query.
 * Two issues have to be addressed:
 *  1) Identify the schema(s) for each operator.
 *  2) Identify the parameters of the operation the operator is executing.
 *     For example, identify the predicate in a SELECT. Or the JOIN PREDICATE.
 */
#include "Catalog.h"
#include "ParseTree.h"
#include "QueryOptimizer.h"
#include "RelOp.h"
#include "EfficientMap.h"
#include "Keyify.h"
#include <typeinfo>

using namespace std;


class QueryCompiler {
private:
	Catalog* catalog;
	QueryOptimizer* optimizer;
	vector<Scan*> ScanMap;
	vector<Select*> SelectMap;
	vector<RelationalOp*> DeleteThis;
	int* _keepMe;

public:
	QueryCompiler(Catalog& _catalog, QueryOptimizer& _optimizer);
	virtual ~QueryCompiler();

	void Compile(TableList* _tables, NameList* _attsToSelect,
		FuncOperator* _finalFunction, AndList* _predicate,
		NameList* _groupingAtts, int& _distinctAtts,
		QueryExecutionTree& _queryTree);
	
	int tableSize(TableList* _tables);//makes it easy to iterate
	RelationalOp* JoinTree(OptimizationTree* root, AndList* _predicate);	//recursive function to create join tree for non-left deep trees
	RelationalOp* GetRelOp(string table);	//goes through select/scan maps to get relational operator associated with table
};

#endif // _QUERY_COMPILER_H
