#include <vector>
#include <unordered_set>

using namespace std; 

//FOR MORE ASSURANCE, USE "STD::SET"
extern vector<unordered_set<int>> node;		//tracks the neighbour of nodes

//degree of a node, (min, max, average) degree of neighbours, sum of the degrees of the neighbors
extern vector<int> deg; 	
extern vector<double> avg_deg, lcc, min_deg, max_deg, sum_nei_degree, deg_elem, deg_set;

void build_graph(vector<vector<int>> &sets, int &nElems);
double smooth(double r); 