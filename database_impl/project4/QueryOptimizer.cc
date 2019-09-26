#include <string>
#include <vector>
#include <iostream>
#include <climits>
#include <limits>

#include "Schema.h"
#include "Comparison.h"
#include "QueryOptimizer.h"
#include "Comparison.h"
#include "EfficientMap.h"
#include "EfficientMap.cc"
#include "Keyify.h"
#include "Keyify.cc"

using namespace std;
OptimizationTree::OptimizationTree() {
	parent = NULL;
	leftChild = NULL;
	rightChild = NULL;
}
void OptimizationTree::Swap(OptimizationTree& _withMe) {
	// Swap vector
	this->tables.swap(_withMe.tables);
	this->tuples.swap(_withMe.tuples);
	
	// Swap noTuples
	int tempI = noTuples;
	this->noTuples = _withMe.noTuples;
	_withMe.noTuples = tempI;
	
	// Swap Order
	string tempS = order;
	this->order = _withMe.order;
	_withMe.order = tempS;

	// Swap Pointer
	OptimizationTree* temp;
	temp = this->parent;
	this->parent = _withMe.parent;
	_withMe.parent = temp;
	temp = this->leftChild;
	this->leftChild = _withMe.leftChild;
	_withMe.leftChild = temp;
	temp = this->rightChild;
	this->rightChild = _withMe.rightChild;
	_withMe.rightChild = temp;
}

void OptimizationTree::CopyFrom(OptimizationTree& _withMe) {
	this->tables = _withMe.tables;
	this->tuples = _withMe.tuples;
	this->noTuples = _withMe.noTuples;
	this->order = _withMe.order;

	this->parent = _withMe.parent;
	this->leftChild = _withMe.leftChild;
	this->rightChild = _withMe.rightChild;
}

QueryOptimizer::QueryOptimizer(Catalog& _catalog) : catalog(&_catalog) {
}

QueryOptimizer::~QueryOptimizer() {
	//cout << "In Query Optimizer destructor" << endl;
	for (int i = 0; i < toBeDelete.size(); ++i) {
		delete toBeDelete[i];
	}
	//cout << "Out of Query Optimizer Destructor" << endl;
}

OptimizationTree* QueryOptimizer::singleNode(string& tName, double & tTuples) {
	// compute some push down tuples size total after cost
	for (int i = 0; i < pushDownList.size(); ++i) {
		if (!tName.compare(pushDownList[i].tableName)) {
			if (pushDownList[i].code == 7) {
				unsigned int temp;
				catalog->GetNoDistinct(tName, pushDownList[i].attName, temp);
				tTuples /= temp;
			}
			else {
				tTuples /= 3;
			}
		}
	}

	OptimizationTree* _root = new OptimizationTree();
	_root->tables.push_back(tName);
	_root->tuples.push_back(tTuples);
	_root->noTuples = 0.0;
	toBeDelete.push_back(_root);

	return _root;
}
/*
 * How it looks in main: 	
 *	OptimizationTree ptr;
 *	optimizer.Optimize(tables, predicate, &ptr);
*/
void QueryOptimizer::Optimize(TableList* _tables, AndList* _predicate,
	OptimizationTree* _root) {
	//cout << _root << endl;
	//std::cout << "Starting Optimize" << std::endl;
	// compute the optimal join order
	//_root = new OptimizationTree();
	OptiMap.Clear();
	costMap.Clear();
	OptimizationTree* tree;
	int size = tableSize(_tables);
	getPredicate(_predicate);
	//cout << _root << endl;
	if (size == 1) {
		double tTuples;
		string tName = _tables->tableName;
		unsigned int temp;
		catalog->GetNoTuples(tName, temp);
		tTuples = temp;
		tree = singleNode(tName, tTuples);
	}
	else if (true) {
	//else if (size == 2) {
		tree = greedy(_tables, _predicate);
	}
	//else {
	//	tree = partition(_tables, _predicate);
	//} 
	_root->Swap(*tree);
}

/*	
 *	Do greedy here
 * 	Implementation refers to optimization.algorithm
 *	This Greedy Algorithm will do the Pre-processing stage and then repeat stage 3 of pre-processing but treating the result as a single table
*/
OptimizationTree* QueryOptimizer::greedy(TableList* _tables, AndList* _predicate) {
	//std::cout << "Starting Greedy " << std::endl;
	OptimizationTree *_root;
	vector<string> currentKey;
	vector<int> popKey;
	int size = tableSize(_tables);
	
	// Pre-processing step 1 : Filling Map with individual table
	TableList* ptrT = _tables;
	Schema* schem;
	//cout << endl;
	//cout << "Starting 1st Preprocessing" << endl;
	while (ptrT != NULL) {
		double tTuples;
		string tName = ptrT->tableName;
		
		unsigned int temp;
		catalog->GetNoTuples(tName, temp);
		tTuples = temp;
		KeyString key(tName);
		currentKey.push_back(tName);
		popKey.push_back(1);
		OptimizationTree toPush;
		
		toPush.tables.push_back(tName);
		toPush.tuples.push_back(tTuples);
		toPush.noTuples = 0.0;
		
		OptiMap.Insert(key, toPush);
		ptrT = ptrT->next;
	}
	
	//cout << "Ending 1st Preprocessing" << endl;
	// Preprocessing step 2 : Push-Down Selections
	//cout << "Starting 2nd Preprocessing" << endl;
	/*
	cout << endl;
	cout << "Primitive tables before applying push-down but before joins" << endl;
	for (int i = 0; i < currentKey.size(); ++i) {
		KeyString key = KeyString(currentKey[i]);
		OptimizationTree* tree = &OptiMap.Find(key);
		cout << "Key " << i << endl;
		cout << "Table Name: " << tree->tables[0] << endl;
		cout << "Tuple Info: " << tree->tuples[0] << endl;
		cout << "Cost  Info: " << tree->noTuples << endl;
	}
	cout << endl;
	*/
	for (int i = 0; i < pushDownList.size(); ++i) {
		KeyString key = KeyString(pushDownList[i].tableName);
		OptimizationTree* ptr = &OptiMap.Find(key);
		//cout << pushDownList[i].tableName << " " << pushDownList[i].code << " " << pushDownList[i].attName << endl;
		if (pushDownList[i].code == 7) {
			unsigned int _noDistinct = 1;
			catalog->GetNoDistinct(pushDownList[i].tableName, pushDownList[i].attName, _noDistinct);
			//cout << "Distinct: " << _noDistinct << endl;
			ptr->tuples[0] /= _noDistinct;
		} else {
			ptr->tuples[0] /= 3;
		}
		//cout << ptr->tables[0] << " " << ptr->tuples[0] << endl;
	}
	//cout << "Ending 2nd Preprocessing" << endl;
	//cout << endl;
	/*
	cout << "Primitive tables after applying push-down but before joins" << endl;
	for (int i = 0; i < currentKey.size(); ++i) {
		KeyString key = KeyString(currentKey[i]);
		OptimizationTree* tree = &OptiMap.Find(key);
		cout << "Key " << i << endl;
		cout << "Table Name: " << tree->tables[0] << endl;
		cout << "Tuple Info: " << tree->tuples[0] << endl;
		cout << "Cost  Info: " << tree->noTuples << endl;
	}
	cout << endl;
	*/
	// Preprocessing step 3 : Join for 2 tables
	//cout << "Starting 3rd Preprocessing" << endl;
	// Only work if table size if greater than 2
	if (size > 1) {
		KeyString key;
		OptimizationTree *optimal, *currentOptimal;
		int startingSize = currentKey.size();
		int J1, J2;
		double noTuples = std::numeric_limits<double>::max();
		for (int i = 0; i < startingSize; ++i) {
			key = KeyString(currentKey[i]);
			currentOptimal = &OptiMap.Find(key);
			for (int j = i + 1; j < startingSize; ++j) {
				//cout << endl;
				key = KeyString(currentKey[j]);
				OptimizationTree* newOptimal = new OptimizationTree();
				OptimizationTree* right = new OptimizationTree();
				OptimizationTree* left = new OptimizationTree();

				newOptimal->CopyFrom(*currentOptimal);
				left->CopyFrom(*currentOptimal);
				right->CopyFrom(OptiMap.Find(key));

				newOptimal->tables.push_back(right->tables[0]);
				newOptimal->tuples.push_back(right->tuples[0]);

				string t1 = newOptimal->tables[0];
				string t2 = newOptimal->tables[1];
				//cout << "Joins cost for table " << t1 << " and table " << t2 << endl;
				newOptimal->noTuples = newOptimal->tuples[0] * newOptimal->tuples[1];
				//cout << "Cost when 2 thing mutiply: " << newOptimal->noTuples << endl;
				for (int k = 0; k < joinList.size(); ++k) {

					string *j1 = &joinList[k].table1;
					string *j2 = &joinList[k].table2;
					unsigned int temp1, temp2;
					if ((!t1.compare(*j1) || !t1.compare(*j2)) && (!t2.compare(*j1) || !t2.compare(*j2))) {
						//cout << "Join conditions exist for the two table on " << joinList[k].att1 << " and " << joinList[k].att2 << endl;
						catalog->GetNoDistinct(*j1, joinList[k].att1, temp1);
						catalog->GetNoDistinct(*j2, joinList[k].att2, temp2);
						unsigned int max = (temp1 > temp2) ? temp1 : temp2;
						//cout << joinList[k].att1 << " : " << temp1 << endl;
						//cout << joinList[k].att2 << " : " << temp2 << endl;
						//cout << "Max is " << max << endl;
						newOptimal->noTuples /= max;
						//cout << "Cost for the joins after applying the division " << newOptimal->noTuples << endl;
					}
				}

				newOptimal->rightChild = right;
				newOptimal->leftChild = left;
				right->parent = newOptimal;
				left->parent = newOptimal;
				if (noTuples > newOptimal->noTuples) {
					optimal = newOptimal;
					noTuples = newOptimal->noTuples;
					J1 = i;
					J2 = j;
				}
				toBeDelete.push_back(right);
				toBeDelete.push_back(newOptimal);
				toBeDelete.push_back(left);
			}
		}

		popKey[J1] = 0;
		popKey[J2] = 0;
		_root = optimal;
		/*
		cout << "Best of Joins of 2 table is: " << endl;
		cout << _root->tables.size() << endl;
		cout << _root->tables[0] << " " << _root->tables[1] << endl;
		cout << _root->noTuples << endl;
		cout << "Ending 3rd Preprocessing" << endl;
		cout << "Computing for Generic Join cases" << endl;
		*/
		// Greedy repeating step 3 ad infinitum
		if (size > 2) {
			for (int i = 0; i < size - 2; ++i) {
				//cout << endl << "Computing joins for " << 3 + i << " tables" << endl;
				noTuples = std::numeric_limits<double>::max();
				
				for (int j = 0; j < popKey.size(); ++j) {
					if (popKey[j]) {
						//cout << "New creation" << endl;
						//cout << endl;
						key = KeyString(currentKey[j]);
						OptimizationTree* newOptimal = new OptimizationTree();
						OptimizationTree* right = new OptimizationTree();

						newOptimal->CopyFrom(*_root);
						right->CopyFrom(OptiMap.Find(key));

						newOptimal->tables.push_back(right->tables[0]);
						newOptimal->tuples.push_back(right->tuples[0]);
						
						newOptimal->rightChild = right;
						right->parent = newOptimal;
						/*
						cout << "Joining ";
						for (int k = 0; k < newOptimal->tables.size(); ++k) {
							cout << newOptimal->tables[k] << " ";
						}
						cout << endl;
						cout << "Current existing cost in newOptimal is " << newOptimal->noTuples << endl;
						// compute cost
						//cout << "Computing cost" << endl;
						*/
						newOptimal->noTuples *= right->tuples[0];
						//cout << "New existing cost in newOptimal after join is " << newOptimal->noTuples << endl;
						for (int a = 0; a < joinList.size(); ++a) {
							string j1 = right->tables[0];
							if (!j1.compare(joinList[a].table1) || !j1.compare(joinList[a].table2)) {
								for (int b = 0; b < newOptimal->tables.size() - 1; ++b) {
									string j2 = newOptimal->tables[b];
									if (!j2.compare(joinList[a].table1) || !j2.compare(joinList[a].table2)) {
										//cout << "Join conditions found for " << j1 << " " << j2 << endl;
										unsigned int temp1, temp2, max;
										catalog->GetNoDistinct(joinList[a].table1, joinList[a].att1, temp1);
										catalog->GetNoDistinct(joinList[a].table2, joinList[a].att2, temp2);
										// << joinList[a].att1 << " " << temp1 << endl;
										//cout << joinList[a].att2 << " " << temp2 << endl;
										max = (temp1 > temp2) ? temp1 : temp2;
										//cout << "Max between the two is: " << max << endl;
										//cout << "Old cost before division " << newOptimal->noTuples << endl;
										newOptimal->noTuples /= max;
										//cout << "New cost after division " << newOptimal->noTuples << endl;
						}	}	}	}
						//cout << "Determining if lowest cost" << endl;
						//cout << noTuples << endl;
						//cout << newOptimal->noTuples << endl;
						//cout << "Current global var join cost" << noTuples << endl;
						if (noTuples > newOptimal->noTuples) {
							//cout << "This join is a better join. Setting optimal Join to this Join" << endl;
							currentOptimal = newOptimal;
							noTuples = newOptimal->noTuples;
							J1 = j;
						}

						toBeDelete.push_back(newOptimal);
						toBeDelete.push_back(right);
					}
					//cout << "Ending inner loop" << j << endl;
				}
				popKey[J1] = 0;
				_root->parent = currentOptimal;
				currentOptimal->leftChild = _root;
				_root = currentOptimal;
				/*
				cout << "Best joins found for " << 3 + i << " tables" << endl;
				for (int k = 0; k < currentOptimal->tables.size(); ++k) {
					cout << "Table " << k << " : " << currentOptimal->tables[0] << endl;
					cout << "Tuples: " << currentOptimal->tuples[0] << endl;
					cout << "Cost: " << currentOptimal->noTuples << endl;
					cout << "Stored in global noTuples: " << noTuples << endl;
				}
				cout << "Ending joins for " << 3 + i << " tables" << endl;
				*/
				//cout << "Ending outer loop" << i << endl;
			}
		}
		//cout << "Ending Greedying Repeat" << endl;
	}

	//cout << "Ending Greedy" << endl;
	//cout << endl<< _root->noTuples << endl;
	//cout << _root << endl;
	/*
	for (int i = 0; i < _root->tables.size(); ++i) {
		cout << _root->tables[i] << " ";
	}
	cout << endl;
	*/
	return _root;
}

OptimizationTree* QueryOptimizer::partition(TableList* _tables, AndList* _predicate) {
	//cout << "In Partition" << endl;
	int size = tableSize(_tables);
	//vector<string> uniqueOrder = getUniqueOrder(_tables, _predicate);
	vector<string> tableKey;
	vector<string> allKey;
	// Pushing single tables to map
	//cout << "Start Preprocessing part 1" << endl;
	TableList* ptrT = _tables;
	while (ptrT != NULL) {
		unsigned int tTuples;
		string tName = ptrT->tableName;

		catalog->GetNoTuples(tName, tTuples);
		KeyString key(tName);
		OptimizationTree toPush;
		KeyDouble data;

		toPush.tables.push_back(tName);
		toPush.tuples.push_back(tTuples);
		toPush.noTuples = 0;
		toPush.order = ptrT->tableName;
		data = KeyDouble(toPush.noTuples);

		key = KeyString(ptrT->tableName);
		tableKey.push_back(tName);

		OptiMap.Insert(key, toPush);
		key = KeyString(ptrT->tableName);
		costMap.Insert(key, data);
		ptrT = ptrT->next;
	}
	//cout << "End Preprocessing part 1" << endl << endl;
	// Push Down Selection
	//cout << "Start Preprocessing part 2" << endl;
	for (int i = 0; i < pushDownList.size(); ++i) {
		KeyString key = KeyString(pushDownList[i].tableName);
		OptimizationTree* ptr = &OptiMap.Find(key);
		if (pushDownList[i].code == 7) {
			unsigned int _noDistinct = 1;
			catalog->GetNoDistinct(pushDownList[i].tableName, pushDownList[i].attName, _noDistinct);
			ptr->tuples[0] /= _noDistinct;
		}
		else {
			ptr->tuples[0] /= 3;
		}
	}
	//cout << "End Preprocessing part 2" << endl << endl;
	// Pushing double tables to map
	//cout << "Starting Preprocessing part 3" << endl;
	allKey = tableKey;
	for (int i = 0; i < size - 1; ++i) {
		for (int j = i + 1; j < size; ++j) {
			OptimizationTree* left = new OptimizationTree();
			OptimizationTree* right = new OptimizationTree();
			OptimizationTree* newJoin = new OptimizationTree();
			KeyString key;
			string toPush;

			key = tableKey[i];
			left->CopyFrom(OptiMap.Find(key));
			key = tableKey[j];
			right->CopyFrom(OptiMap.Find(key));
			newJoin->CopyFrom(*left);

			newJoin->tables.push_back(right->tables[0]);
			newJoin->tuples.push_back(right->tuples[0]);
			toPush = '(' + allKey[i] + '|' + allKey[j] + ')';
			newJoin->order = toPush;

			newJoin->noTuples = newJoin->tuples[0] * newJoin->tuples[1];
			//cout << newJoin->noTuples << endl;
			for (int k = 0; k < joinList.size(); ++k) {
				string *j1 = &joinList[k].table1;
				string *j2 = &joinList[k].table2;
				string *t1 = &newJoin->tables[0];
				string *t2 = &newJoin->tables[1];
				unsigned int temp1, temp2, max;
				if ((!t1->compare(*j1) || !t1->compare(*j2)) && (!t2->compare(*j1) || !t2->compare(*j2))) {
					catalog->GetNoDistinct(*j1, joinList[k].att1, temp1);
					catalog->GetNoDistinct(*j2, joinList[k].att2, temp2);
					//cout << joinList[k].att1 << " " << temp1 << endl;
					//cout << joinList[k].att2 << " " << temp2 << endl;
					max = (temp1 > temp2) ? temp1 : temp2;
					newJoin->noTuples /= max;
			}	}
			toBeDelete.push_back(left);
			toBeDelete.push_back(right);
			toBeDelete.push_back(newJoin);

			
			allKey.push_back(toPush);
			tableKey.push_back(toPush);
			key = KeyString(toPush);
			OptiMap.Insert(key, *newJoin);
		}
	}
	
	cout << "Everything in OptiMap" << endl;
	for (int i = 0; i < tableKey.size(); ++i) {
		KeyString key = KeyString(tableKey[i]);
		OptimizationTree* data = &OptiMap.Find(key);
		cout << endl << "Order: " << data->order << " Cost: " << data->noTuples << endl;
		for (int j = 0; j < data->tables.size(); ++j) {
			cout << data->tables[j] << " " << data->tuples[j] << " ";
		}
	}
	
	
	//cout << "End Preprocessing part 3" << endl;
	//cout << "Start Best Join Computation" << endl;
	// Algorithm
	string optimalString;
	double noTuples = std::numeric_limits<double>::max();
	vector<string> uniqueOrdering = getUniqueOrder(_tables, _predicate);
	/* 
	for (int i = 0; i < uniqueOrdering.size(); ++i) {
		cout << uniqueOrdering[i] << endl;
	}
	*/
	cout << endl << endl;;
	for (int i = 0; i < uniqueOrdering.size(); ++i) {
		cout << uniqueOrdering[i] << endl;
	}
	cout << endl;
	for (int i = 0; i < uniqueOrdering.size(); ++i) {
		vector<joinOrder> joinOrdering = getJoinOrder(uniqueOrdering[i], size);
		string temp;
		double noTuples2 = 0;
		for (int j = 0; j < joinOrdering.size(); ++j) {
			cout << "j1 " << joinOrdering[j].j1 << " j2 " << joinOrdering[j].j2 << endl;
		}
		for (int j = 0; j < joinOrdering.size(); ++j) {
			cout << endl;
			OptimizationTree *left, *right, *ptr;
			KeyString key;
			string toPush;
			bool copy = false;
			toPush = '(' + joinOrdering[j].j1 + joinOrdering[j].j2 + ')';
			key = KeyString(joinOrdering[j].j1);

			if (OptiMap.IsThere(key)) {
				left = &OptiMap.Find(key);
			}
			else {
				cout << "Left not found" << endl;
				cout << "j1: " << joinOrdering[j].j1 << endl;
				cout << "j2: " << joinOrdering[j].j2 << endl;
				cout << allKey[allKey.size() - 1] << endl;
			}
			key = KeyString(joinOrdering[j].j2);
			if (OptiMap.IsThere(key)) {
				right = &OptiMap.Find(key);
			}
			else {
				cout << "Right not found" << endl; 
				cout << "j1: " << joinOrdering[j].j1 << endl;
				cout << "j2: " << joinOrdering[j].j2 << endl;
				cout << allKey[allKey.size() - 1] << endl;
			}
			toPush = '(' + joinOrdering[j].j1 + joinOrdering[j].j2 + ')';
			key = KeyString(toPush);
			KeyString toCheck = KeyString('(' + joinOrdering[j].j1 + '|' + joinOrdering[j].j2 + ')');
			if (OptiMap.IsThere(key) || OptiMap.IsThere(toCheck)) {
				copy = true;
				//cout << "ptr exist in OptiMap" << endl;
				key = KeyString('(' + joinOrdering[j].j1 + joinOrdering[j].j2 + ')');
				if (OptiMap.IsThere(key)) {
					//cout << "ptr Order: " << '(' + joinOrdering[j].j1 + joinOrdering[j].j2 + ')' << endl;
					ptr = &OptiMap.Find(key);
				}
				key = KeyString('(' + joinOrdering[j].j1 + '|' + joinOrdering[j].j2 + ')');
				if (OptiMap.IsThere(key)) {
					//cout << "ptr Order: " << '(' + joinOrdering[j].j1 + '|' + joinOrdering[j].j2 + ')' << endl;
					ptr = &OptiMap.Find(key);
				}
			}
			else {
				cout << "ptr not exist in OptiMap" << endl;
				ptr = new OptimizationTree();
				toBeDelete.push_back(ptr);
				ptr->CopyFrom(*right);
				ptr->order = '(' + joinOrdering[j].j1 + joinOrdering[j].j2 + ')';
				//cout << "ptr Order: " << ptr->order << endl;
				for (int k = 0; k < left->tables.size(); ++k) {
					ptr->tables.push_back(left->tables[k]);
					ptr->tuples.push_back(left->tuples[k]);
				}
				if (ptr->noTuples * left->noTuples != 0)
					ptr->noTuples *= left->noTuples;
			}

			// compute Join cost 
			//cout << "Outside Join" << endl;
			if (!copy) {
				cout << "i " << i << " j " << j << endl;
				for (int w = 0; w < joinList.size(); ++w) {
					string *j1 = &joinList[w].table1;
					string *j2 = &joinList[w].table2;
					for (int x = 0; x < left->tables.size(); ++x) {
						string *t1 = &left->tables[x];
						if (!t1->compare(*j1) || !t1->compare(*j2)) {
							for (int y = 0; y < right->tables.size(); ++y) {
								string *t2 = &right->tables[y];
								if (!t2->compare(*j1) || !t2->compare(*j2)) {
									unsigned int temp1, temp2, max;
									//cout << "j1 " << *j1 << " j2 " << *j2 << endl;
									//cout << "t1 " << *t1 << " t2 " << *t2 << endl;
									catalog->GetNoDistinct(*j1, joinList[w].att1, temp1);
									catalog->GetNoDistinct(*j2, joinList[w].att2, temp2);
									//cout << joinList[w].att1 << " " << temp1 << endl;
									//cout << joinList[w].att2 << " " << temp2 << endl;
									max = (temp1 > temp2) ? temp1 : temp2;
									//cout << "Max is " << max << endl;
									//cout << "Cost before performing Join condition " << ptr->noTuples << endl;
									ptr->noTuples /= max;
									//cout << "Cost after performing Join condition " << ptr->noTuples << endl;
								}
							}
						}
					}
				}
			}
			temp = ptr->order;
			//cout << "End Join" << endl;
			//cout << "noTuples2 before " << noTuples2 << endl;
			if (j != joinOrdering.size() - 1)
				noTuples2 += ptr->noTuples;
			// Push new thing into efficientMap
			//cout << "noTuples2 after " << noTuples2 << endl;
			if (!copy) {
				tableKey.push_back(toPush);
				key = KeyString(toPush);
				allKey.push_back(toPush);
				OptiMap.Insert(key, *ptr);
			}
		}
		if (noTuples > noTuples2) {
			noTuples = noTuples2;
			optimalString = temp;
		}
	}
	//cout << "End Best Join Computation" << endl;
	// Optimal Join Orders is stored in optimalString
	
	cout << "OptimalString is " << optimalString << endl;
	/*
	cout << "Everything in OptiMap" << endl;
	for (int i = 0; i < tableKey.size(); ++i) {
		KeyString key = KeyString(tableKey[i]);
		OptimizationTree* data = &OptiMap.Find(key);
		cout << endl << "Order: " << data->order << " Cost: " << data->noTuples << endl;
		for (int j = 0; j < data->tables.size(); ++j) {
			cout << data->tables[j] << " " << data->tuples[j] << " ";
		}
	}
	*/
	
	return NULL;
	//cout << "Start tree creation" << endl;
	vector<joinOrder> joinOrdering = getJoinOrder(optimalString, size);
	OptimizationTree* _root;
	//cout << joinOrdering.size()<< endl;
	for (int i = 0; i < joinOrdering.size(); ++i) {
		//cout << "i " << i << endl;
		KeyString key;
		OptimizationTree *left, *right, *ptr;

		//cout << "Start Left" << endl;
		key = KeyString(joinOrdering[i].j1);
		if (OptiMap.IsThere(key))
			left = &OptiMap.Find(key);
		else
			cout << "Left not found" << endl;
		//cout << "Start Right" << endl;
		key = KeyString(joinOrdering[i].j2);
		if (OptiMap.IsThere(key))
			right = &OptiMap.Find(key);
		else
			cout << "right not found" << endl;
		//cout << "Start ptr" << endl;
		key = KeyString('(' + joinOrdering[i].j1 + joinOrdering[i].j2 + ')');
		if (OptiMap.IsThere(key))
			ptr = &OptiMap.Find(key);
		else {
			cout << "ptr not found" << endl;
			cout << '(' + joinOrdering[i].j1 + joinOrdering[i].j2 + ')' << endl;
		}
		//cout << "End ptr" << endl;

		//cout << "Do ptr" << endl;
		ptr->leftChild = left;
		//cout << "Do 2 ptr" << endl;
		ptr->rightChild = right;
		//cout << "Do 3 ptr" << endl;
		left->parent = ptr;
		//cout << "Do 4 ptr" << endl;
		right->parent = ptr;
		//cout << "Do 5 ptr" << endl;
		//cout << "pointers done" << endl;
		_root = ptr;
		//cout << "Do 6 ptr " << endl;
	}
	/*
	for (int i = 0; i < _root->tables.size(); ++i) {
		cout << _root->tables[i] << endl;
	}
	*/
	return _root;
	
}

int QueryOptimizer::tableSize(TableList* _tables) {
	int size = 0;
	TableList* ptr = _tables;
	while (ptr != NULL) {
		ptr = ptr->next;
		++size;
	}
	return size;
}

void QueryOptimizer::getPredicate(AndList* _predicate) {
	pushDownList.clear();
	joinList.clear();

	AndList* myPredicate = _predicate;

	while (myPredicate != NULL) {

		ComparisonOp* myComp = myPredicate->left;

		if (myComp != NULL) {

			if (myComp->left->code == 3 && myComp->right->code != 3) {

				pushDown myPushObj;

				myPushObj.code = myComp->code;

				myPushObj.attName = myComp->left->value;

				string temp = myComp->left->value;

				myPushObj.tableName = findTableName(temp);

				pushDownList.push_back(myPushObj);

			}

			else if (myComp->left->code != 3 && myComp->right->code == 3) {

				pushDown myPushObj;

				myPushObj.code = myComp->code;

				myPushObj.attName = myComp->right->value;

				string temp = myComp->right->value;

				myPushObj.tableName = findTableName(temp);

				pushDownList.push_back(myPushObj);

			}

			else if (myComp->left->code == 3 && myComp->right->code == 3) {

				joinStuff myJoinObj;

				myJoinObj.att1 = myComp->left->value;
				myJoinObj.att2 = myComp->right->value;

				string temp = myComp->left->value;

				myJoinObj.table1 = findTableName(temp);

				temp = myComp->right->value;

				myJoinObj.table2 = findTableName(temp);

				joinList.push_back(myJoinObj);

			}


		}

		myPredicate = myPredicate->rightAnd;

	}

	// --------------------PRINTING LISTS-------------------
	/*

	for (int i = 0; i < pushDownList.size(); i++) {

	cout << pushDownList[i].attName;

	if (pushDownList[i].code == 5)
	cout << " < ";
	else if (pushDownList[i].code == 6)
	cout << " > ";
	else if (pushDownList[i].code == 7)
	cout << " = ";

	cout << "Some Constant" << endl;
	cout << pushDownList[i].tableName << endl;

	}

	for (int i = 0; i < joinList.size(); i++) {

	cout << joinList[i].att1 << " = " << joinList[i].att2 << endl;
	cout << joinList[i].table1 << " and " << joinList[i].table2 << endl;

	}
	*/

}

string QueryOptimizer::findTableName(string& attName) {

	for (int i = 0; i < catalog->currentTables.size(); i++) {

		vector<string> myAtts;

		catalog->GetAttributes(catalog->currentTables[i], myAtts);

		for (int j = 0; j < myAtts.size(); j++) {

			if (myAtts[j] == attName) {

				return catalog->currentTables[i];

			}

		}

		myAtts.clear();

	}

}




// --- Partition ---

bool dupeCheck(string& str1, string& str2) {



	for (int i = 0; i < str1.length(); i++) {

		char letter = str1.at(i);

		if (letter >= 'A' && letter <= 'Z') {

			for (int j = 0; j < str2.length(); j++) {

				if (letter == str2.at(j)) {

					return true;

				}

			}

		}

	}

}


vector<string> QueryOptimizer::getUniqueOrder(TableList* _tables, AndList* _predicate) {
	
	vector<string> letters;

	letters.push_back("A"); letters.push_back("B"); letters.push_back("C"); letters.push_back("D"); letters.push_back("E");
	letters.push_back("F"); letters.push_back("G"); letters.push_back("H"); letters.push_back("I"); letters.push_back("J");

	TableList* temp = _tables;

	vector<string> myTables;

	int tSize = tableSize(_tables);

	for (int i = 0; i < tSize; i++) {

		KeyString key = KeyString(letters[i]);
		KeyString data = KeyString(temp->tableName);

		tableMap.Insert(key, data);

		temp = temp->next;

	}


	//cout << endl << "Tables:" << endl;

	for (int i = 0; i < tSize; i++) {

		myTables.push_back(letters[i]);

	}

	/*for (int i = 0; i < tSize; i++) {

		cout << myTables[i] << endl;

	}

	cout << endl;*/


	vector<pattern> myPatterns;

	for (int i = 3; i <= tSize; i++) {

		pattern newPattern;

		vector<int> newLeft;

		for (int j = 1; j <= i/2; j++) {

			newLeft.push_back(j);

		}

		vector<int> newRight;

		for (int j = i-1; j >= (i+1)/2; j--) {

			newRight.push_back(j);

		}

		newPattern.left = newLeft;
		newPattern.right = newRight;

		myPatterns.push_back(newPattern);

	}

	
	/*for (int i = 0; i < myPatterns.size(); i++) {

		cout << "To make: " << 3 + i << endl;

		cout << "Left: ";

		for (int j = 0; j < myPatterns[i].left.size(); j++) {

			cout << myPatterns[i].left[j];

		}

		cout << endl << "Right: ";

		for (int j = 0; j < myPatterns[i].right.size(); j++) {

			cout << myPatterns[i].right[j];

		}

		cout << endl << endl;

	}*/
	



	vector < vector < string > > myCombs;

	// Column 1

	vector <string> row1;

	for (int i = 0; i < tSize; i++) {

		row1.push_back(myTables[i]);

	}

	myCombs.push_back(row1);

	// Column 2

	vector <string> row2;

	for (int i = 0; i < myCombs[0].size(); i++) {

		for (int j = i; j < myCombs[0].size(); j++) {

			if (myCombs[0][i].find(myCombs[0][j]) > 1000000) {

				string temp;
				temp.append("(").append(myCombs[0][i]).append("|").append(myCombs[0][j]).append(")");
				row2.push_back(temp);

			}

		}

	}

	myCombs.push_back(row2);

	//cout << myPatterns.size() << endl;

	for (int i = 0; i < myPatterns.size(); i++) {

		vector <string> row;

		//cout << "Here" << endl;

		for (int j = 0; j < myPatterns[i].left.size(); j++) {

			//cout << "Here2" << endl;
			
			for (int l = 0; l < myCombs[myPatterns[i].left[j] - 1].size(); l++) {
				
				//cout << "Here3" << endl;
				//cout << myPatterns[i].left[j] << " And " << myPatterns[i].right[j] << endl;
				int r = 0;

				if (myPatterns[i].left[j] == myPatterns[i].right[j]) {
					//cout << "Here4" << endl;
					r = l;

				}

				for (r; r < myCombs[myPatterns[i].right[j] - 1].size(); r++) {

					//cout << "Comparing: " << myCombs[myPatterns[i].left[j] - 1][l] << " AND " << myCombs[myPatterns[i].right[j] - 1][r] << endl;

					if (!dupeCheck(myCombs[myPatterns[i].right[j] - 1][r], myCombs[myPatterns[i].left[j] - 1][l])) {

						//cout << "Left not found in Right" << endl;

						string temp;
						temp.append("(").append(myCombs[myPatterns[i].left[j] - 1][l]).append(myCombs[myPatterns[i].right[j] - 1][r]).append(")");
						row.push_back(temp);

					}

				}

			}

		}

		myCombs.push_back(row);

	}


	//cout << endl << endl;

	
	/*for (int i = 0; i < myCombs.size(); i++) {

		cout << "Column: " << i + 1 << endl;

		for (int j = 0; j < myCombs[i].size(); j++) {

			cout << myCombs[i][j] << endl;

		}

		cout << endl;

	}*/
	
	return myCombs[myCombs.size() - 1];
}


vector<joinOrder> QueryOptimizer::getJoinOrder(string str, int& tSize) {
	// use ptr like a normal vector. Just -> instead of .
	vector<joinOrder> myOrder;

	// (A(B(C|D)))
	// ((A|B)(C|D))

	//C D
	//B CD
	//A BCD

	int i = str.length()-1;
	vector<int> closeIndex;
	vector<int> openIndex;

	//cout << str << endl;

	// Pass 1

	while (i > 0) {

		if (str.at(i) == '|') {

			joinOrder newOrder;

			newOrder.j1 = str.at(i - 1);
			newOrder.j2 = str.at(i + 1);

			//cout << newOrder.j1 << " " << newOrder.j2 << endl;

			myOrder.push_back(newOrder);

		}

		i--;

	}



	// (A(B(C|D)))
	// ((A|B)(C|D))

	// Pass 2

	i = str.length() - 1;

	while (i > 0) {

		if (str.at(i) == ')') {

			closeIndex.insert(closeIndex.begin(), i);

		}

		else if (str.at(i) == '(') {

			joinOrder newOrder;

			if (str.at(i - 1) != '(' && str.at(i - 1) != ')') {

				newOrder.j1 = str.at(i - 1);
				newOrder.j2 = str.substr(i, closeIndex[0] - i + 1);
				closeIndex.erase(closeIndex.begin());

				// cout << newOrder.j1 << " " << newOrder.j2 << endl;

				myOrder.push_back(newOrder);
				i--;
				continue;

			}

			else {

				openIndex.insert(openIndex.begin(), i);

				if (openIndex.size() == 2) {

					newOrder.j1 = str.substr(openIndex[0], closeIndex[0] - openIndex[0] + 1);
					openIndex.erase(openIndex.begin());
					closeIndex.erase(closeIndex.begin());
					newOrder.j2 = str.substr(openIndex[0], closeIndex[0] - openIndex[0] + 1);
					openIndex.erase(openIndex.begin());
					closeIndex.erase(closeIndex.begin());

					// cout << newOrder.j1 << " " << newOrder.j2 << endl;

					myOrder.push_back(newOrder);
					i--;
					continue;

				}

				else {

					i--;
					continue;

				}

			}

			//cout << newOrder.j1 << " " << newOrder.j2 << endl;

		}

		i--;

	}

	//cout << endl;

	// vector will delete joinOrder so they dont have to be a pointer
	// create joinOrder from str
	// push_back smaller joins before larger joins. e.g. ( (A|B) (C|D) E) will have (A B), (C D), before (AB CD) and then (ABCD E) in the push_back. 
	


	for (int i = 0; i < myOrder.size(); i++) {

		for (int j = 0; j < myOrder[i].j1.length(); j++) {

			char letter = myOrder[i].j1.at(j);
			
			if (letter >= 'A' && letter <= 'Z') {

				string temp;

				temp += letter;

				KeyString myKey = KeyString(temp);

				string tName = tableMap.Find(myKey);

				myOrder[i].j1.erase(j, 1);

				myOrder[i].j1.insert(j, tName);

				//cout << myOrder[i].j1 << endl;

			}

		}

		for (int j = 0; j < myOrder[i].j2.length(); j++) {

			char letter = myOrder[i].j2.at(j);

			if (letter >= 'A' && letter <= 'Z') {

				string temp;

				temp += letter;

				KeyString myKey = KeyString(temp);

				string tName = tableMap.Find(myKey);

				myOrder[i].j2.erase(j, 1);

				myOrder[i].j2.insert(j, tName);

				//cout << myOrder[i].j2 << endl;

			}

		}

	}


	return myOrder;
}