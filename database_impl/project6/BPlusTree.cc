#include "BPlusTree.h"

using namespace std;

BNode::BNode(int size) {
	keyCount = 0;
	key = new int[size];
}

BNode::~BNode() {
	delete[] key;
}

void BNode::print() {
	cout << "Key Count is " << keyCount << endl;
	cout << "Keys:      ";
	for (int i = 0; i < keyCount; ++i) {
		cout << key[i] << " ";
	}
	cout << endl;
}

internalNode::internalNode(int size) : BNode(size) {
	pageNum = new int[size + 1];
	children = new BNode*[size + 1];
	type = INTERNAL;
	parent = NULL;
	childrenCount = 0;
}

internalNode::~internalNode() {
	delete[] pageNum;
	for (int i = 0; i < childrenCount; ++i) {
		delete children[i];
	}
	delete[] children;
}

void internalNode::print() {
	cout << "Internal Node" << endl;
	BNode::print();
	for (int i = 0; i < childrenCount; ++i) {
		cout << "Internal Node Child: " << i << " pageNum: " << pageNum[i] << " Type: ";
		children[i]->print();
		cout << endl;
	}
}

leafNode::RecordInfo::RecordInfo(int size) {
	pageNum = new int[size];
	recordNum = new int[size];
}

leafNode::RecordInfo::~RecordInfo() {
	delete[] pageNum;
	delete[] recordNum;
}

leafNode::leafNode(int size) : BNode(size) {
	info = new RecordInfo(size);
	next = NULL;
	parent = NULL;
	type = LEAF;
}

leafNode::~leafNode() {
	delete info;
}

void leafNode::print() {
	cout << "Leaf Node" << endl;
	BNode::print();
	/*
	cout << "pageNum:   ";
	for (int i = 0; i < keyCount; ++i) {
	cout << info->pageNum[i] << " ";
	}
	cout << endl;
	cout << "recordNum: ";
	for (int i = 0; i < keyCount; ++i) {
	cout << info->recordNum[i] << " ";
	}
	cout << endl;
	*/
}

BPlusTree::BPlusTree(int numKey) {
	this->numKey = numKey;
	pageCount = 1;
	root = new leafNode(numKey);
	height = 1;
}
BPlusTree::~BPlusTree() {
	delete root;
}
leafNode* BPlusTree::Find(int key, BNode* node, int& keyIndex) {
	//cout << "node type is " << ((node->type == INTERNAL) ? "INTERNAL " : "LEAF ") << endl;
	// Recursive case
	if (node->type == INTERNAL) {
		internalNode* temp = (internalNode*)node;
		// Handles the case that key is less/equal then existing key in the node
		for (int i = 0; i < temp->keyCount; ++i) {
			//cout << "Iteration " << i << endl;
			if (key < temp->key[i]) {
				//	cout << "key <= temp->key[i] " << key << " <= " << temp->key[i] << endl;
				keyIndex = i;
				return Find(key, temp->children[i], keyIndex);
			}

			else if (key == temp->key[i]) {
				keyIndex = i;
				return Find(key, temp->children[i + 1], keyIndex);
			}

		}
		// Handle the case that key is greater then existing key
		keyIndex = temp->keyCount;
		//cout << "Other case" << endl;
		return Find(key, temp->children[temp->keyCount], keyIndex);
	}
	// Base case
	else if (node->type == LEAF) {
		return (leafNode*)node;
	}
}

int BPlusTree::SplitInternal(internalNode* node) {
	if (node->keyCount < numKey) {
		internalNode* newNode = new internalNode(numKey);
		newNode->parent = node->parent;

		return 1;
	}
	return 0;
}

void BPlusTree::Connect(internalNode* parent, internalNode* child) {
	/*
	cout << "PARENT AHUASHIUASHAIUSDGHIUDG" << endl;
	if (parent != NULL) parent->print();
	cout << "CHILD ASUIHUAISHASUH" << endl;
	child->print();
	*/
	if (parent == NULL) {
		cout << "root key " << root->keyCount << endl;
		/*
		for (int i = 0; i < root-; ++i) {
		cout << root->key[i] << " ";
		}
		cout << endl;
		*/
		cout << "BASHBAIUSBAI" << endl;
		if ((root->keyCount) != numKey) {
			cout << "Splitting Root in Connect" << endl;
			internalNode* childFriend = (internalNode*)root;
			internalNode* newRoot = new internalNode(numKey);
			// Appoint New Root
			newRoot->key[0] = child->key[0];
			newRoot->keyCount += 1;

			for (int i = 0; i < child->keyCount - 1; ++i) {
				child->key[i] = child->key[i + 1];
			}
			child->keyCount -= 1;
			// Connect the root
			newRoot->children[0] = childFriend;
			newRoot->children[1] = child;
			newRoot->childrenCount += 2;

			/*
			int minCap = numKey / 2;
			newRoot->key[0] = childFriend->key[minCap];
			newRoot->keyCount += 1;
			internalNode* newParent = new internalNode(numKey);
			newParent->children[0] = childFriend->children[minCap + 1];
			for (int i = 1; i < minCap; ++i) {
			newParent->key[i - 1] = childFriend->key[minCap + i];
			newParent->children[i] = childFriend->children[minCap + i + 1];
			}
			newParent->childrenCount = minCap;
			newParent->keyCount = minCap - 1;
			childFriend->childrenCount -= minCap;
			childFriend->keyCount -= minCap;
			return;
			// Connect the root
			newRoot->children[0] = childFriend;
			newRoot->children[1] = newParent;
			newRoot->childrenCount += 2;
			// Add Child into Root
			Insert((BNode*)child, newRoot);
			*/
			root = newRoot;
			//root->print();
			height++;
		}
		else {
			cout << "IN HERE" << endl;
			internalNode* rootIntern = (internalNode*)root;
			rootIntern->key[rootIntern->keyCount] = child->key[0];
			rootIntern->keyCount++;

			for (int i = 0; i < child->keyCount - 1; ++i) {
				child->key[i] = child->key[i + 1];
			}
			child->keyCount -= 1;

			rootIntern->children[rootIntern->childrenCount] = child;
			rootIntern->childrenCount += 1;
		}
	}
	else {
		// Parent is not Full Base Case
		if (parent->keyCount != numKey) {
			// Parent is not Full Base Case
			parent->key[parent->keyCount] = child->key[0];
			parent->keyCount += 1;
			parent->children[parent->childrenCount] = child;
			parent->childrenCount += 1;
			// Need to remove key[0] from child
			for (int i = 0; i < child->keyCount - 1; ++i) {
				child->key[i] = child->key[i + 1];
			}
			child->keyCount -= 1;
			child->key[child->keyCount] = 0;
			// Sort the Parent so its key are sorted
			for (int i = parent->keyCount - 1; i > 0; --i) {
				if (parent->key[i] < parent->key[i - 1]) {
					// Swap Key
					int tempKey = parent->key[i];
					parent->key[i] = parent->key[i - 1];
					parent->key[i - 1] = tempKey;
					// Swap Children
					BNode* tempChild = parent->children[i + 1];
					parent->children[i + 1] = parent->children[i];
					parent->children[i] = tempChild;
					// Most likely need to update pointers?
				}
				else {
					break;
				}
			}
		}
		// Parent is Full Recursion Occurs
		else {
			// Split the parent
			internalNode* newParent = new internalNode(numKey);

			int minCap = numKey / 2;
			for (int i = 0; i < minCap; ++i) {
				newParent->key[i] = parent->key[i + minCap + numKey % 2];
				newParent->pageNum[i] = parent->pageNum[i + minCap + 1 + numKey % 2];
				newParent->children[i] = parent->children[i + minCap + 1 + numKey % 2];
			}
			newParent->childrenCount = minCap;
			newParent->keyCount = minCap;
			parent->childrenCount -= minCap;
			parent->keyCount -= minCap;
			// Insert child into one of the parent
			if (child->key[0] <= parent->key[0]) {
				Insert((BNode*)child, parent);
			}
			else {
				Insert((BNode*)child, newParent);
			}
			// Need to connect the two parents with its parents
			Connect((internalNode*)parent->parent, newParent);
		}
	}
}

int BPlusTree::Insert(int key, int pageNum, int recordNum) {
	// Handle the beginning case where the root also acts as a leafNode
	if (root->type == LEAF) {
		// Leaf is not full
		if (root->keyCount < numKey) {
			// Insert First Element has no comparison
			//Insert((leafNode*)root, key, pageNum, recordNum);

			if (root->keyCount == 0) {
				root->key[root->keyCount] = key;
				((leafNode*)root)->info->pageNum[root->keyCount] = pageNum;
				((leafNode*)root)->info->recordNum[root->keyCount] = recordNum;
				root->keyCount += 1;
			}

			else {
				Insert((leafNode*)root, key, pageNum, recordNum);
			}
		}
		// Leaf is full, Need to split, and change root so its an internalNode
		else {
			internalNode* temp = new internalNode(numKey);
			leafNode* newLeaf = new leafNode(numKey);
			leafNode* rootTemp = (leafNode*)root;
			// Calculate how many Node to steal and steal Node from rootTemp
			int minCap = numKey / 2;
			for (int i = 0; i < minCap; ++i) {
				newLeaf->key[i] = rootTemp->key[i + minCap + numKey % 2];
				newLeaf->info->pageNum[i] = rootTemp->info->pageNum[i + minCap + numKey % 2];
				newLeaf->info->recordNum[i] = rootTemp->info->recordNum[i + minCap + numKey % 2];
				newLeaf->keyCount += 1;
			}
			rootTemp->keyCount -= minCap;

			// Key Belongs to newLeaf
			if (key >= newLeaf->key[0]) {
				Insert(newLeaf, key, pageNum, recordNum);
			}
			// Key belongs to rootTemp
			else {
				Insert(rootTemp, key, pageNum, recordNum);
			}

			// Connect the nodes
			temp->children[0] = rootTemp;
			temp->children[1] = newLeaf;
			temp->pageNum[0] = pageCount++;
			temp->pageNum[1] = pageCount++;
			temp->childrenCount = 2;

			temp->key[0] = newLeaf->key[0];
			temp->keyCount++;

			rootTemp->parent = temp;
			newLeaf->parent = temp;
			rootTemp->next = newLeaf;
			root = temp;
			height++;
		}
	}
	else if (root->type == INTERNAL) {
		// Check which leafNode to insert to
		int toInsertIndex = -1;
		leafNode* toInsert = Find(key, root, toInsertIndex);
		// Safe to Insert
		if (toInsert->keyCount < numKey) {
			return Insert(toInsert, key, pageNum, recordNum);
		}
		// Need to split
		else {
			//return 1;
			//cout << "Need to split" << endl;
			internalNode* parent = (internalNode*) toInsert->parent;
			leafNode* newLeaf = new leafNode(numKey);
			newLeaf->parent = parent;

			// Calculate how many Node to steal and steal Node from toInsert
			int minCap = numKey / 2;
			for (int i = 0; i < minCap; ++i) {
				newLeaf->key[i] = toInsert->key[i + minCap + numKey % 2];
				newLeaf->info->pageNum[i] = toInsert->info->pageNum[i + minCap + numKey % 2];
				newLeaf->info->recordNum[i] = toInsert->info->recordNum[i + minCap + numKey % 2];
				newLeaf->keyCount += 1;
			}
			toInsert->keyCount -= minCap;


			if (key >= newLeaf->key[0]) {
				Insert(newLeaf, key, pageNum, recordNum);
			}
			else {
				Insert(toInsert, key, pageNum, recordNum);
			}
			// Safe to update
			if (parent->keyCount < numKey) {
				// Insert 
				parent->children[parent->childrenCount] = newLeaf;
				parent->pageNum[parent->childrenCount] = pageCount++;
				parent->key[parent->keyCount] = newLeaf->key[0];
				parent->childrenCount += 1;
				parent->keyCount += 1;
				// Insertion Sort Time
				int index = parent->keyCount;
				for (int i = parent->keyCount - 1; i > 0; --i) {
					if (parent->key[i] < parent->key[i - 1]) {
						// Swap
						int temp1;
						BNode* temp2;
						// Keys
						temp1 = parent->key[i];
						parent->key[i] = parent->key[i - 1];
						parent->key[i - 1] = temp1;
						// Children Pointers
						temp2 = parent->children[i + 1];
						parent->children[i + 1] = parent->children[i];
						parent->children[i] = temp2;
						index = i;
					}
					else {
						// Sorted Do Not Need To Continue
						break;
					}
				}
				// Update the Leaf Next pointers
				if (index == parent->childrenCount - 1) {
					((leafNode*)parent->children[index])->next = ((leafNode*)parent->children[index - 1])->next;
					((leafNode*)parent->children[index - 1])->next = (leafNode*)parent->children[index];
				}
				else {
					((leafNode*)parent->children[index])->next = (leafNode*)parent->children[index + 1];
					((leafNode*)parent->children[index - 1])->next = (leafNode*)parent->children[index];
				}
			}
			// Need to split parent
			else {
				internalNode* temp = parent;
				internalNode* newParent = new internalNode(numKey);
				int minCap = (numKey / 2);
				for (int i = 0; i < minCap; ++i) {
					newParent->key[i] = parent->key[i + minCap + numKey % 2];
					newParent->pageNum[i] = parent->pageNum[i + minCap + 1 + numKey % 2];
					newParent->children[i] = parent->children[i + minCap + 1 + numKey % 2];
				}
				newParent->childrenCount = minCap;
				newParent->keyCount = minCap;
				parent->childrenCount -= minCap;
				parent->keyCount -= minCap;

				/*
				cout << "Printing parent" << endl;
				parent->print();
				cout << "Printing new Parent" << endl;
				newParent->print();

				return 1;
				*/

				// Insert newLeaf into one of the two internalNode
				if (newLeaf->key[0] <= parent->key[0]) {
					//cout << "left" << endl;
					Insert(newLeaf, parent);
					//parent->print();
				}
				else {
					//cout << "right" << endl;
					Insert(newLeaf, newParent);
					//newParent->print();
				}
				// Connect them
				Connect((internalNode*)parent->parent, newParent);
			}

		}
	}
	return 1;
}

int BPlusTree::Insert(BNode* child, internalNode* parent) {
	// Insert leaf into internal
	//cout << "At the Start" << endl;
	//parent->print();
	//cout << 1 << endl;
	parent->key[parent->keyCount] = child->key[0];
	parent->children[parent->childrenCount] = child;
	parent->keyCount += 1;
	parent->childrenCount += 1;
	child->parent = parent;
	//cout << 2 << endl;
	// Insertion Sort the keys
	int index = parent->keyCount - 1;
	for (int i = parent->keyCount - 1; i > 0; --i) {
		if (parent->key[i] < parent->key[i - 1]) {
			cout << "IN LOOP " << i << endl;
			// Keys
			int temp = parent->key[i];
			parent->key[i] = parent->key[i - 1];
			parent->key[i - 1] = temp;
			// Children Pointers
			BNode* temp2 = parent->children[i + 1];
			parent->children[i + 1] = parent->children[i];
			parent->children[i] = temp2;
			index = i;
		}
		else {
			break;
		}
	}
	//cout << 3 << endl;
	// Update next pointers if children are leafs
	if (child->type == LEAF) {
		if (index == parent->childrenCount - 1) {
			cout << "left" << endl;
			((leafNode*)parent->children[index])->next = ((leafNode*)parent->children[index - 1])->next;
			((leafNode*)parent->children[index - 1])->next = (leafNode*)parent->children[index];
		}
		else {
			cout << "right" << endl;
			((leafNode*)parent->children[index])->next = (leafNode*)parent->children[index + 1];
			((leafNode*)parent->children[index - 1])->next = (leafNode*)parent->children[index];
		}
	}
	return 1;
}

/*	Insert functions currently allows duplicate key to exist
To prevent duplicate key from existing, In the Loop of // Figure out where to put it
add an if statement that checks if the key is the same and don't insert if it does exist
*/
int BPlusTree::Insert(leafNode* leaf, int key, int pageNum, int recordNum) {
	// Figure out where to put it
	int index = -1;
	for (int i = 0; i < leaf->keyCount; ++i) {
		if (leaf->key[i] <= key) {
			index = i;
		}
		else {
			break;
		}
	}
	// Insert to Right
	if (index == leaf->keyCount - 1) {
		leaf->key[leaf->keyCount] = key;
		leaf->info->pageNum[leaf->keyCount] = pageNum;
		leaf->info->recordNum[leaf->keyCount] = recordNum;
		leaf->keyCount += 1;
	}
	// Insert to Left
	else {
		// Insert new stuff to the end of array
		leaf->key[leaf->keyCount] = key;
		leaf->info->pageNum[leaf->keyCount] = pageNum;
		leaf->info->recordNum[leaf->keyCount] = recordNum;
		// Shift last element using insertion sort
		for (int i = leaf->keyCount; i > index + 1; --i) {
			int temp;
			// Keys
			if (leaf->key[i] < leaf->key[i - 1]) {
				temp = leaf->key[i];
				leaf->key[i] = leaf->key[i - 1];
				leaf->key[i - 1] = temp;
				// pageNum
				temp = leaf->info->pageNum[i];
				leaf->info->pageNum[i] = leaf->info->pageNum[i - 1];
				leaf->info->pageNum[i - 1] = temp;
				// recordNum
				temp = leaf->info->recordNum[i];
				leaf->info->recordNum[i] = leaf->info->recordNum[i - 1];
				leaf->info->recordNum[i - 1] = temp;
			}
			else {
				break;
			}
		}
		leaf->keyCount += 1;
	}
	return 1;
}
/* FOR HANG DO NOT DELETE
DO NOT DELETE
DO NOT DELETE
int index = -1;
for (int i = 0; i < root->keyCount; ++i) {
if (root->key[i] <= key) {
index = i;
}
}
leafNode* rootTemp = ((leafNode*)root);
// Insert to right
if (index == root->keyCount - 1) {
root->key[root->keyCount] = key;
rootTemp->info->pageNum[root->keyCount] = pageNum;
rootTemp->info->recordNum[root->keyCount] = recordNum;
root->keyCount += 1;
}
// Insert to left
else {
// Insert new stuff to end of array
rootTemp->key[root->keyCount] = key;
rootTemp->info->pageNum[root->keyCount] = pageNum;
rootTemp->info->recordNum[root->keyCount] = recordNum;
// Shift last element using insertion sort
for (int i = rootTemp->keyCount; i > index + 1; --i) {
int temp;
// Keys
temp = rootTemp->key[i];
rootTemp->key[i] = root->key[i - 1];
rootTemp->key[i - 1] = temp;
// pageNum
temp = rootTemp->info->pageNum[i];
rootTemp->info->pageNum[i] = rootTemp->info->pageNum[i - 1];
rootTemp->info->pageNum[i - 1] = temp;
// recordNum
temp = rootTemp->info->recordNum[i];
rootTemp->info->recordNum[i] = rootTemp->info->recordNum[i - 1];
rootTemp->info->recordNum[i - 1] = temp;

}
rootTemp->keyCount += 1;

*/
int BPlusTree::Find(int key, int& pageNum, int& recNum) { 

	int i = 0;
	int x;

	leafNode * temp = Find(key, root, x);

	for (int i = 0; i < temp->keyCount; i++) {
		if (key == temp->key[i]) { 
			*temp->info->pageNum = pageNum;
			*temp->info->recordNum = recNum;
			return 1;
		}
	}

	return 0;	
}

void BPlusTree::print() {
	cout << "Printing from Root" << endl;
	cout << "Height is " << height << endl;
	//cout << "Number of Key " << numKey << endl;
	root->print();

}
void BPlusTree::printLeaf() {
	BNode* temp = root;
	while (temp->type != LEAF) {
		temp = ((internalNode*)temp)->children[0];
	}
	leafNode* leaf = (leafNode*)temp;
	do {
		cout << "Leaf Node with KeyCount " << leaf->keyCount << endl;
		cout << "Keys ";
		for (int i = 0; i < leaf->keyCount; ++i) {
			cout << leaf->key[i] << " ";
		}
		cout << endl;
		leaf = leaf->next;
	} while (leaf != NULL);
}
int BPlusTree::writeToDisk(IndexFile* file, Schema iNode, Schema lNode)
{
	int lastType, lastParent;
	print();
	//cout << "Inside writeToDisk" << endl;
	traverseAndWrite(file, iNode, lNode, root, -1, lastType, lastParent);
	//cout << "Out of traverseAndWrite" << endl;
	file->AppendLastIndex(lastType, lastParent);
	file->CloseIndex();
}

template <typename T>
string convert(T x)
{
	ostringstream convert;   			// stream used for the conversion
	convert << x;		      			// insert the textual representation of 'Number' in the characters in the stream
	return convert.str(); 				// set 'Result' to the contents of the stream
};

int BPlusTree::traverseAndWrite(IndexFile* file, Schema iNode, Schema lNode, BNode * node, int parent, int& lastType, int& lastParent)
{
	//cout << "In traversAndWrite" << endl;
	// traverse the node
	if (node->type == LEAF) {
		// Write
		for (int i = 0; i < node->keyCount; ++i) {
			int ikey = node->key[i];
			int dpnum = ((leafNode*)node)->info->pageNum[i];
			int recnum = ((leafNode*)node)->info->recordNum[i];
			string str = convert(ikey) + convert('|') + convert(dpnum) + convert('|') + convert(recnum) + convert('|');
			char* text = new char[str.length() + 1];
			strcpy(text, str.c_str());
			FILE* fp;
			fp = fmemopen(text, str.length() * sizeof(char), "r");
			Record recTemp;
			recTemp.ExtractNextRecord(lNode, *fp);
			fclose(fp);
			delete text;
			int type = 1;	// means Leaf
			//cout << "About to AppendRecordIndex" << endl;
			file->AppendRecordIndex(recTemp, type, parent);
		}
		lastType = LEAF;
		lastParent = parent;
		return 1;
	}
	else if (node->type == INTERNAL) {
		// Write
		internalNode* temp = (internalNode*)node;
		for (int i = 0; i < node->keyCount; ++i)
		{
			int ikey = node->key[i];
			int cpnum = temp->pageNum[i];
			string str = convert(ikey) + convert('|') + convert(cpnum) + convert('|');
			char* text = new char[str.length() + 1];
			strcpy(text, str.c_str());
			FILE* fp;
			fp = fmemopen(text, str.length() * sizeof(char), "r");
			Record recTemp;
			recTemp.ExtractNextRecord(iNode, *fp);
			fclose(fp);
			delete text;
			int type = 0;	// means Leaf
			file->AppendRecordIndex(recTemp, type, parent);
		}
		//temp->recordNum;
		for (int i = 0; i < temp->childrenCount; ++i) {
			traverseAndWrite(file, iNode, lNode, (temp->children)[i], (temp->pageNum)[i], lastType, lastParent);
		}
		lastType = INTERNAL;
		lastParent = parent;
	}
}
