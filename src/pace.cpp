#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

#include "graph.h"
#include "highs.h"
#include "netkit.h"
#include "mlpredict.h"
#include "prune.h"
#include "nusc.h"

#define el <<"\n"
#define sp <<" "


using namespace std;

int nElems, updElem, updSet;
vector<vector<int>> sets, notun; 
vector<unordered_set<int>> member;
unordered_map<int, int> mapp, rmapp, setMap, rsetMap;
vector<int> res; 
unordered_set<int> final;

void loadInp(){
	string dumm, str;
	int s;
	getline(cin, str);
	{
		stringstream ss(str); 
		ss>>dumm>>dumm>>nElems>>s; 
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
	build_graph(sets, nElems);
	solve_hitting_set(sets, nElems, t_limit);
	int n = deg.size();
	cout<<fixed <<setprecision(6);
	for(int i = 1; i < n; i++){
		//cout<<i<<"\t";
		cout<<deg_elem[i]<<",";
		cout<<deg_set[i]<<",";
		cout<<min_deg[i]<<","; 
		cout<<max_deg[i]<<",";
		cout<<sum_nei_degree[i]<<",";
		cout<<avg_deg[i]<<","; 
		cout<<smooth(lcc[i])<<",";
		cout<<smooth(btwScores[i - 1])<<",";
		cout<<smooth(clsScores[i - 1])<<",";
		cout<<coreNumbers[i - 1]<<",";
		cout<<included[i]<<"\n";
	}
}

vector<float> get_vector(int r){
	vector<float> ans = {
		to_float(deg_elem[r]), to_float(deg_set[r]), to_float(min_deg[r]), to_float(max_deg[r]), to_float(sum_nei_degree[r]), 
		to_float(avg_deg[r]), to_float(smooth(lcc[r])), to_float(smooth(btwScores[r - 1])), to_float(smooth(clsScores[r - 1])),
		to_float(coreNumbers[r])
	};
	return ans; 
}

void do_mapping() {
    int nSets = sets.size();

    updSet = 0;
    int set_idx = 1;
    for (int i = 0; i < nSets; i++) {
        if (!delSet[i]) {
            setMap[i] = set_idx;
            rsetMap[set_idx] = i;
            set_idx++;
            updSet++;
        }
    }

    updElem = 0;
    int elem_idx = 1;
    for (int i = 1; i <= nElems; i++) {
        if (!delElem[i]) {
            vector<int> temp;
            for (int r : member[i]) {
                if (!delSet[r]) {
                    temp.push_back(setMap[r]);  // Now safe
                }
            }

            if (!temp.empty()) {
                notun.push_back(temp);
                mapp[i] = elem_idx;
                rmapp[elem_idx] = i;
                elem_idx++;
                updElem++;
            }
        }
    }
}

void show(){
	//for(int r: final) cout<<r sp; cout el; cout el; 
	cout<<updElem sp<<updSet el; 
	for(auto v: notun){
		cout<<v.size() el; 
		for(int r: v) cout<<r sp; cout el; 
	}
}

void mergeFromNuSC() {
    for (int chosen_new_set_idx : res) {
        auto it = rsetMap.find(chosen_new_set_idx);
        if (it != rsetMap.end()) {
            int original_set_idx = it->second;
            for (int original_elem_id : sets[original_set_idx]) {
                if (!delElem[original_elem_id]) {
                    final.insert(original_elem_id);
                }
            }
        }
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
	    
	double lim = 250; 
    float one_thresh = 0.65, zero_thresh = 0.6;

    MLPredictor model("../random_forest_model.onnx");
    for(int i = 1; i <= nElems; i++){
        vector<float> input_vec = get_vector(i);
        PredictionResult result = model.predict(input_vec);
        pair<int, float> pp = {i, result.probabilities[result.label]};
        //cout<<result.label<<","<<pp.second el;
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
	run_nusc(notun, res, updElem, updSet, remaining); 
	mergeFromNuSC();
	//solve_hitting_set(notun, updElem, remaining);
	printResult();
	//produce_tc(notun, updElem, updSet); 
	//for(int i = 1; i <= nElems; i++) if(!delElem[i]) final.insert(i);
	//printResult();
	return; 
}

/*
	auto start = std::chrono::high_resolution_clock::now();
	
	auto finish = std::chrono::high_resolution_clock::now();
	double secs = std::chrono::duration<double>(finish - start).count();
	cout<<"time taken = "<<secs<<" seconds\n";	
*/
