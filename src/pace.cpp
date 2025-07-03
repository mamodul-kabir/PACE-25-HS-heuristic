#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

#include "graph.h"
#include "netkit.h"
#include "mlpredict.h"
#include "prune.h"
#include "nusc.h"

#define el <<"\n"
#define sp <<" "


using namespace std;

int nElems, nSets, updElem, updSet;
vector<vector<int>> sets, notun; 
vector<unordered_set<int>> member;
unordered_map<int, int> mapp, rmapp;
vector<int> res, inc; 
unordered_set<int> final;

void loadInp(){
	string dumm, str;
	getline(cin, str);
	{
		stringstream ss(str); 
		ss>>dumm>>dumm>>nElems>>nSets; 
	}
	member.resize(nElems + 1); 
	int ind = 0;
	while(getline(cin, str)){
		stringstream ss(str);
		int x; 
		vector<int> temp;
		while(ss>>x){
			//x = (x == n ? 0 : x); 
			temp.push_back(x);
			member[x].insert(ind); 
		}
		sets.push_back(temp);
		ind++; 
	}
}

template <typename T>
float to_float(T value) {
    return static_cast<float>(value);
}

void generate_csv(double t_limit){
	loadInp();
	auto start = std::chrono::high_resolution_clock::now();
	build_graph(sets, nElems);

	auto finish = std::chrono::high_resolution_clock::now();
	double secs = std::chrono::duration<double>(finish - start).count();
	cerr<<"time taken = "<<secs<<endl; 
	int lim = floor(t_limit);
	run_nusc(sets, res, nSets, nElems, lim);

	inc.resize(nElems + 1, 0); 
	for(int r: res) inc[r] = 1;  

	cout<<fixed <<setprecision(6);
	for(int i = 1; i <= nElems; i++){
		//cout<<i<<"\t";
		cout<<deg_elem[i]<<",";
		cout<<deg_set[i]<<",";
		cout<<min_deg[i]<<","; 
		cout<<max_deg[i]<<",";
		cout<<sum_nei_degree[i]<<",";
		cout<<avg_deg[i]<<","; 
		cout<<smooth(lcc[i])<<",";
		cout<<coreNumbers[i - 1]<<",";
		cout<<inc[i]<<"\n";
	}
}

vector<float> get_vector(int r){
	vector<float> ans = {
		to_float(deg_elem[r]), to_float(deg_set[r]), to_float(min_deg[r]), to_float(max_deg[r]), to_float(sum_nei_degree[r]), 
		to_float(avg_deg[r]), to_float(smooth(lcc[r])), to_float(coreNumbers[r])
	};
	return ans; 
}


void do_mapping(){
	updElem = 0; 
	int idx = 1; 
	for(int i = 1; i <= nElems; i++){
		if(!delElem[i]){
			mapp[i] = idx; 
			rmapp[idx] = i; 
			idx++; 
			updElem++; 
		}
	}

    updSet = 0;
    for (int i = 0; i < nSets; i++) {
        if (!delSet[i]) {
            vector<int> temp;
            for(int r: sets[i]){
            	if(!delElem[r]){
            		temp.push_back(mapp[r]); 
            	}
            }
            notun.push_back(temp); 
            updSet++; 
        }
    }
}

void show(){
	cout<<updElem sp<<updSet el; 
	for(auto v: notun){
		cout<<v.size() el; 
		for(int r: v) cout<<r sp; cout el; 
	}
}


void mergeFromNuSC() {
    for (int r : res) {
    	final.insert(rmapp[r]);         
    }
}

void printResult(){
	cout<<final.size() el; 
	for(int r: final) cout<<r el; 
	return; 
}

void init(){
	loadInp(); 
	auto start = std::chrono::high_resolution_clock::now();
	build_graph(sets, nElems); 
	    
	double lim = 295; 
    float one_thresh = 0.95, zero_thresh = 0.95;

    MLPredictor model("../rf_model.onnx");
    for(int i = 1; i <= nElems; i++){
        vector<float> input_vec = get_vector(i);
        PredictionResult result = model.predict(input_vec);
        pair<int, float> pp = {i, result.probabilities[result.label]};
        if(result.label == 0 && pp.second >= zero_thresh) add_zero(pp);
        else if(result.label == 1 && pp.second >= one_thresh) add_one(pp); 
    }
    cerr<<"prediction done\n";
    prune(sets, member, final, nElems); 
    do_mapping();
   	cerr<<"pruning and mapping done\n"; 
    auto finish = std::chrono::high_resolution_clock::now();
	double secs = std::chrono::duration<double>(finish - start).count();
	int remaining =  max((int ) floor(lim - secs), 15);
	//show();
	cerr<<"running nusc for "<<remaining<<" seconds\n"; 
	run_nusc(notun, res, updSet, updElem, remaining); 
	mergeFromNuSC();
	printResult();
	return; 
}
