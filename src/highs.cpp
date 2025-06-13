#include "Highs.h"

#include <vector>
#include <iostream>
#include <unordered_set>

using namespace std; 

vector<double> solution; 
vector<int> included;

int solve_hitting_set(const vector<vector<int>> &sets, const int &nElems, double t_limit) {
    Highs highs;

    //included.resize(nElems + 1);
    const int num_vars = nElems;             // Number of elements = number of variables
    const int num_constraints = sets.size(); // Each set defines a constraint

    vector<double> col_cost(num_vars, 1.0);         // Minimize sum of x_i
    vector<double> col_lower(num_vars, 0.0);        // x_i >= 0
    vector<double> col_upper(num_vars, 1.0);        // x_i <= 1 (binary)
    vector<HighsVarType> integrality(num_vars, HighsVarType::kInteger);

    // Build sparse matrix in column-wise format (CSC)
    vector<HighsInt> A_start(num_vars + 1, 0);
    vector<HighsInt> A_index;
    vector<double> A_value;

    vector<unordered_set<int>> var_to_constraints(num_vars);
    for (int row = 0; row < num_constraints; ++row) {
        for (int col : sets[row]) {
            var_to_constraints[col - 1].insert(row); // assuming 1-based indexing
        }
    }

    int nz = 0;
    for (int col = 0; col < num_vars; ++col) {
        A_start[col] = nz;
        for (int row : var_to_constraints[col]) {
            A_index.push_back(row);
            A_value.push_back(1.0);
            ++nz;
        }
    }
    A_start[num_vars] = nz;

    vector<double> row_lower(num_constraints, 1.0);       // each set must be hit
    vector<double> row_upper(num_constraints, kHighsInf); // no upper bound

    HighsModel model;
    model.lp_.num_col_ = num_vars;
    model.lp_.num_row_ = num_constraints;
    model.lp_.col_cost_ = col_cost;
    model.lp_.col_lower_ = col_lower;
    model.lp_.col_upper_ = col_upper;
    model.lp_.row_lower_ = row_lower;
    model.lp_.row_upper_ = row_upper;
    model.lp_.a_matrix_.format_ = MatrixFormat::kColwise;
    model.lp_.a_matrix_.start_ = A_start;
    model.lp_.a_matrix_.index_ = A_index;
    model.lp_.a_matrix_.value_ = A_value;
    model.lp_.integrality_ = integrality;

    // Set a time limit (e.g., 60 seconds)
    highs.setOptionValue("time_limit", t_limit);
    highs.setOptionValue("mip_rel_gap", 0.005);
    highs.setOptionValue("output_flag", false); 

    highs.passModel(model);
    highs.run();

    HighsModelStatus status = highs.getModelStatus();
    if (status == HighsModelStatus::kOptimal || status == HighsModelStatus::kTimeLimit) {
        const vector<double>& solution = highs.getSolution().col_value;
        //cout << "Optimal (or best found) hitting set: ";
        for (int i = 0; i < solution.size(); ++i) {
            if (solution[i] > 0.5) included.push_back(i + 1);
        }
        //cerr << "Objective value: " << highs.getObjectiveValue() << endl;
    } else {
        cerr << "No feasible solution found within the time limit.\n";
    }

    return 0;
}