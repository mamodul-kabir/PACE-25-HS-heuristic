#include <utility>
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_set>

using namespace std;

vector<pair<int, float>> ones, zeros;
vector<int> setSize;
vector<bool> delSet, delElem;

void add_zero(pair<int,float> r){
	zeros.push_back(r); 
}
void add_one(pair<int,float> r){
	ones.push_back(r); 
}

bool compareDesc(const std::pair<int, float>& a, const std::pair<int, float>& b) {
    return a.second > b.second;
}

void prune(const vector<vector<int>> &sets, const vector<unordered_set<int>> &member, unordered_set<int> &final, const int &nElems){
	sort(zeros.begin(), zeros.end(), compareDesc);

	int nSets = sets.size(); 
	setSize.resize(nSets); 
	delSet.resize(nSets, false);
	delElem.resize(nElems + 1, false);

	for(int i = 0; i < nSets; i++) setSize[i] = sets[i].size();
	for(auto r: ones){
		delElem[r.first] = 1;
		final.insert(r.first); 
		for(int p: member[r.first]){
			delSet[p] = 1; 
		}
	}

	for(auto r: zeros){
		int breaker = -1; 
		bool becomesEmpty = false; 
		vector<int> changed_sets;
		for (int p : member[r.first]) {
		    if (delSet[p]) continue;
		    if (setSize[p] <= 3) {
		        becomesEmpty = true;
		        breaker = p;
		        break;
		    } else {
		        setSize[p] -= 1;
		        changed_sets.push_back(p);
		    }
		}
		if (becomesEmpty) {
		    for (int p : changed_sets) setSize[p] += 1;
		} else {
		    delElem[r.first] = 1;
		}
	}
}

void dekhao(){
	cout<<zeros.size()<<" "<<ones.size()<<endl;
}
