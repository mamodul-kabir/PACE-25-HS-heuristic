#include "wscp.h"
#include <ctime>

using namespace std; 

void produce_tc(vector<vector<int>> &notun, int &updElem, int &updSet){
    cout<<updElem<<" "<<updSet<<"\n";
    for(int i = 0; i < updSet; i++) cout<<"1 ";
    cout<<"\n"; 
    for(int i = 0; i < updElem; i++){
        int nS = notun[i].size(); 
        cout<<nS<<endl; 
        for(int j = 0; j < nS; j++){
            cout<<notun[i][j]<<" "; 
        }
        cout<<endl; 
    }

}

void run_nusc(vector<vector<int>> &notun, vector<int> &res, int &updElem, int &updSet, int &remaining){
    double var_num = (double) updElem, set_num = (double) updSet;

    int time_limit = remaining;
    WSCP wscp_solver(time_limit);
    int seed = time(NULL);           
    srand(seed);

    int new_weight, tabu_len;
    double novelty_p;

    if (set_num > var_num){
            new_weight = 5;
            tabu_len = 4;
            novelty_p = 0.1;
    }else{
        new_weight = 80;
        tabu_len = 5;
        if(var_num/set_num > 10)
            novelty_p = 0.05;
        else novelty_p = 0.5;
    }

    wscp_solver.build_instance(notun, updElem, updSet);
    cerr<<"NuSC build: complete\n";
    wscp_solver.reduce_instance();
    cerr<<"NuSC reduction: complete\n";
    wscp_solver.set_param(new_weight, tabu_len, novelty_p);
    
    start_timing();
    wscp_solver.init();
    cerr<<"NuSC init: complete\n";
    wscp_solver.local_search();
    cerr<<"NuSC local_search: complete\n";
    wscp_solver.check_solu();

    for (int i = 0; i < wscp_solver.set_num; ++i){
        if(wscp_solver.best_solu[i] == 1){
            res.push_back(i + 1);
        }
    }
    wscp_solver.free_memory();
}
