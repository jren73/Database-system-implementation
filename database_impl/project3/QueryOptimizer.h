#ifndef _QUERY_OPTIMIZER_H
#define _QUERY_OPTIMIZER_H

#include "Schema.h"
#include "Catalog.h"
#include "ParseTree.h"
#include "RelOp.h"
#include "EfficientMap.h"
#include "InefficientMap.h"
#include "Keyify.h" 

#include <string>
#include <vector>

using namespace std;


// data structure used by the optimizer to compute join ordering
struct OptimizationTree {
	public:
	// list of tables joined up to this node
	vector<string> tables;
	// number of tuples in each of the tables (after selection predicates)
	vector<double> tuples;
	// number of tuples at this node
	double noTuples;
	// Join order
	string order;

	//default constructor
	OptimizationTree();	

	// connections to children and parent
	OptimizationTree* parent;
	OptimizationTree* leftChild;
	OptimizationTree* rightChild;
	
	// Swap and CopyFrom for EfficientMap
	void Swap(OptimizationTree& _withMe);
	void CopyFrom(OptimizationTree& _withMe);
};

struct pushDown {
	string tableName;
	int code;				// 5 >
							// 6 < 
							// 7 =
	string attName;				
};

struct joinStuff {
	string table1;
	string att1;
	string table2;
	string att2;
};

struct pattern {
	vector<int> left;
	vector<int> right;
};

struct joinOrder {
	string j1;
	string j2;
};

class QueryOptimizer {
private:
	Catalog* catalog;
	// EfficientMap 
	EfficientMap<KeyString, OptimizationTree> OptiMap;
	EfficientMap<KeyString, KeyDouble> costMap;
	// Deconstructor stuff, push in new OptimizationTree
	vector<OptimizationTree*> toBeDelete;
	
	// AndList Stuff
	vector<pushDown>  pushDownList;
	vector<joinStuff> joinList;

	// Get Join Orders given a string
	vector<joinOrder> getJoinOrder(string str, int& tSize);
	//EfficientMap<KeyString, OptimizationTree> OptiMap;
private:
	EfficientMap<KeyString, KeyString> tableMap;

	int tableSize(TableList* _tables);

	void treeTraversal(OptimizationTree *node);

	//OptimizationTree* createJoin(OptimizationTree* j1, OptimizationTree* j2, EfficientMap<KeyString, );
	OptimizationTree* greedy(TableList* _tables, AndList* _predicate);
	OptimizationTree* partition(TableList* _tables, AndList* _predicate);
	// Utility Functions
	string findTableName(string& attName);
	void getPredicate(AndList* _predicate);
	OptimizationTree* singleNode(string& tName, double& tTuples);			// Create a single Opti Tree Nodes and return address
public:
	QueryOptimizer(Catalog& _catalog);
	virtual ~QueryOptimizer();
	void Optimize(TableList* _tables, AndList* _predicate, OptimizationTree* _root);
	vector<string> getUniqueOrder(TableList* _tables, AndList* _predicate);
};

#endif // _QUERY_OPTIMIZER_H
