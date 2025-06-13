#include <utility>
#include <vector>
#include <unordered_set>

using namespace std; 
extern vector<bool> delSet, delElem;

void add_zero(pair<int,float> r);
void add_one(pair<int,float> r);
bool compareDesc(const pair<int, float>& a, const pair<int, float>& b);
void prune(const vector<vector<int>> &sets, const vector<unordered_set<int>> &member, unordered_set<int> &final, const int &nElems); 

void dekhao();
