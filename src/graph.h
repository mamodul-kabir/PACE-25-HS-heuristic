#include <vector>
#include <unordered_set>

using namespace std; 

extern vector<unordered_set<int>> node;		

extern vector<int> deg; 	
extern vector<double> avg_deg, lcc, min_deg, max_deg, sum_nei_degree, deg_elem, deg_set;

void build_graph(vector<vector<int>> &sets, int &nElems);
double smooth(double r); 