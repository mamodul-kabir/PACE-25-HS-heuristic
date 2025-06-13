#include "graph.h"
#include "netkit.h"

#include <vector>
#include <iostream>
#include <unordered_set>
#include <queue>
#include <stack>
#include <iomanip>
#include <cmath>

using namespace std;

vector<unordered_set<int>> node;		

vector<int> deg; 	
vector<double> avg_deg, lcc, min_deg, max_deg, sum_nei_degree, deg_elem, deg_set;

double norma = 0.0;

void build_graph(vector<vector<int>> &sets, int &nElems){
	node.resize(nElems + 1); 
	deg.resize(nElems + 1);
	avg_deg.resize(nElems + 1);
	min_deg.resize(nElems + 1);
	max_deg.resize(nElems + 1);
	sum_nei_degree.resize(nElems + 1);
	lcc.resize(nElems + 1);
	deg_elem.resize(nElems + 1);
	deg_set.resize(nElems + 1);
	for(int i = 1; i < nElems; i++) G.addNode();
	int nSets = sets.size();

	for(auto v: sets){
		int ssize = v.size();
		for(int i = 0; i < ssize; i++){
			deg[v[i]] += 1;
			for(int j = i + 1; j < ssize; j++){
				node[v[i]].insert(v[j]);
				node[v[j]].insert(v[i]); 
				G.addEdge(v[i] - 1, v[j] - 1);
			}
		}
	}	
	for(int i = 1; i <= nElems; i++){
		int mn = nElems + 100, mx = -1;
		//for each i, 'tri' tracks the pair of neighbors j-k s.t. j-k are neighbours, it is required to find Local Clustering Coefficient
		int tri = 0; 	
		int nn = node[i].size(); 			//number of neighbors of node i
		norma += deg[i];

		for(int j: node[i]){
			mn = min(mn, deg[j]);
			mx = max(mx, deg[j]); 
			sum_nei_degree[i] += deg[j];
			//*
			for(int k: node[j]){
				if(k == i) continue; 
				if(node[i].count(k)) tri++;
			}
			//*/
		}
		tri >>= 1;
		min_deg[i] = (mn == (nElems + 100) ? -1 : mn);
		max_deg[i] = mx; 
		avg_deg[i] = (nn > 0 ? (((double) sum_nei_degree[i])/nn) : -1);
		lcc[i] = (nn < 2 ? 0.0 : (((double) 2 * tri)/(nn * (nn - 1)))); 
	}

	norma /= nElems;
	
	for (int i = 1; i <= nElems; i++) {
		deg_elem[i] = ((double)deg[i]) / nElems;
		deg_set[i] = ((double)deg[i]) / nSets;

		if (min_deg[i] != -1) min_deg[i] /= norma;
		if (max_deg[i] != -1) max_deg[i] /= norma;
		if (avg_deg[i] != -1) avg_deg[i] /= norma;

		if (!node[i].empty())
			sum_nei_degree[i] = sum_nei_degree[i] / (node[i].size() * norma);
		else
			sum_nei_degree[i] = -1;

		lcc[i] = smooth(lcc[i]);
	}

	run();
}

double smooth(double r){
	if (abs(r) < 1e-12 || (isnan(abs(r)))) r = 0.0;
	return r;
}