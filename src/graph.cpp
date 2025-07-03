#include "graph.h"
#include "netkit.h"

#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <iomanip>
#include <cmath>
#include <algorithm>

using namespace std;

vector<vector<int>> neighbors;

vector<int> deg;
vector<double> avg_deg, lcc, min_deg, max_deg, sum_nei_degree, deg_elem, deg_set;

vector<array<int, 2>> dnode_min2, dnode_max2;
vector<int> dnode_sum;

double norma = 0.0;

inline double smooth(double r) {
	if (abs(r) < 1e-12 || (isnan(abs(r)))) r = 0.0;
	return r;
}

void build_graph(vector<vector<int>> &sets, int &nElems) {
	int nSets = sets.size();
	int totalNodes = nElems + nSets;

	neighbors.clear(); neighbors.resize(totalNodes + 2);  
	deg.clear(); deg.resize(totalNodes + 2);
	avg_deg.resize(nElems + 2);
	min_deg.resize(nElems + 2);
	max_deg.resize(nElems + 2);
	sum_nei_degree.resize(nElems + 2);
	lcc.resize(nElems + 2);
	deg_elem.resize(nElems + 2);
	deg_set.resize(nElems + 2);
	dnode_min2.assign(nSets + 1, {INT_MAX, INT_MAX});
	dnode_max2.assign(nSets + 1, {INT_MIN, INT_MIN});
	dnode_sum.assign(nSets + 1, 0);

	G = NetworKit::Graph(totalNodes, false, false);  

	for (int id = 1; id <= nSets; ++id) {
		int dnode = nElems + id;
		for (int v : sets[id - 1]) {
			neighbors[v].push_back(dnode);
			neighbors[dnode].push_back(v);
			deg[v]++;
			deg[dnode]++;
			G.addEdge(v - 1, dnode - 1); 

			int d = deg[v];
			dnode_sum[id] += d;

			if (d < dnode_min2[id][0]) {
				dnode_min2[id][1] = dnode_min2[id][0];
				dnode_min2[id][0] = d;
			} else if (d < dnode_min2[id][1]) {
				dnode_min2[id][1] = d;
			}

			if (d > dnode_max2[id][0]) {
				dnode_max2[id][1] = dnode_max2[id][0];
				dnode_max2[id][0] = d;
			} else if (d > dnode_max2[id][1]) {
				dnode_max2[id][1] = d;
			}
		}
	}

	norma = 0.0;
	for (int i = 1; i <= nElems; ++i) norma += deg[i];
	norma /= nElems;

	for (int x = 1; x <= nElems; ++x) {
		int triangles = 0;
		int m = 0;
		for (int d : neighbors[x]) {
			int sz = neighbors[d].size();
			int contrib = sz - 1;
			if (contrib >= 1) {
				m += contrib;
				triangles += (contrib * (contrib - 1)) / 2;
			}
		}
		lcc[x] = (m < 2 ? 0.0 : ((double)triangles) / (m * (m - 1) / 2));
		lcc[x] = smooth(lcc[x]);
	}

	for (int i = 1; i <= nElems; ++i) {
		int mn = INT_MAX, mx = -1, sum_d = 0, cnt = 0;

		for (int d : neighbors[i]) {
			int id = d - nElems;
			int sz = neighbors[d].size();
			if (sz <= 1) continue;

			int dsum = dnode_sum[id];
			int local_min = (deg[i] == dnode_min2[id][0]) ? dnode_min2[id][1] : dnode_min2[id][0];
			int local_max = (deg[i] == dnode_max2[id][0]) ? dnode_max2[id][1] : dnode_max2[id][0];
			int others = sz - 1;

			cnt += others;
			sum_d += (dsum - deg[i]);
			mn = min(mn, local_min);
			mx = max(mx, local_max);
		}

		min_deg[i] = (mn == INT_MAX ? -1 : (double)mn / norma);
		max_deg[i] = (mx == -1 ? -1 : (double)mx / norma);
		avg_deg[i] = (cnt == 0 ? -1 : ((double)sum_d / cnt / norma));
		sum_nei_degree[i] = (cnt == 0 ? -1 : ((double)sum_d / (cnt * norma)));
	}


	for (int i = 1; i <= nElems; ++i) {
		deg_elem[i] = ((double)deg[i]) / nElems;
		deg_set[i] = ((double)deg[i]) / nSets;
	}

	run(); 
	cerr << "feature calculation done\n";
}
