/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file test_kkt/KktChStep.cpp
 * @brief
 */
#include "test_kkt/KktCh2.h"

#include <cassert>
#include <utility>

#include "test_kkt/DevKkt.h"

using std::cout;
using std::endl;
using std::get;
using std::pair;
using std::setw;
using std::vector;

namespace presolve {

namespace dev_kkt_check {

void KktChStep::setBoundsCostRHS(const vector<double>& colUpper_,
                                 const vector<double>& colLower_,
                                 const vector<double>& cost,
                                 const vector<double>& rowLower_,
                                 const vector<double>& rowUpper_) {
  RcolLower = colLower_;
  RcolUpper = colUpper_;
  RrowLower = rowLower_;
  RrowUpper = rowUpper_;
  RcolCost = cost;
}

void KktChStep::addCost(HighsInt col, double value) { RcolCost[col] = value; }

/*
1  SING_ROW
17 DOUBLETON_EQUATION

171 DOUBLETON_EQUATION_ROW_BOUNDS_UPDATE
173 DOUBLETON_EQUATION_X_ZERO_INITIALLY
172 DOUBLETON_EQUATION_NEW_X_NONZERO
174 DOUBLETON_EQUATION_NEW_X_ZERO_AR_UPDATE
175 DOUBLETON_EQUATION_NEW_X_ZERO_A_UPDATE

7 FIXED_COL
0 EMPTY_ROW
6 EMPTY_COL
9 DOMINATED_COLS
10 WEAKLY_DOMINATED_COLS
4 FREE_SING_COL
5 SING_COL_DOUBLETON_INEQ
19 SING_COL_DOUBLETON_INEQ_SECOND_SING_COL
8 IMPLIED_FREE_SING_COL
3 FORCING_ROW
2 FORCING_ROW_VARIABLE
16 REDUNDANT_ROW

*/
void KktChStep::addChange(int type, HighsInt row, HighsInt col, double valC,
                          double dualC, double dualR) {
  // when updating fill new values for b, c, bounds in Rb RcolCost RcolUpper,
  // RcolLower
  vector<pair<HighsInt, double>> upd;

  switch (type) {
    case 171:  // new bounds from doubleton equation, retrieve old ones
      upd = rLowers.top();
      rLowers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RrowLower[ind] = get<1>(upd[i]);
      }
      upd = rUppers.top();
      rUppers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RrowUpper[ind] = get<1>(upd[i]);
      }
      break;
    case 1:  // row singleton
      upd = cLowers.top();
      cLowers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolLower[ind] = get<1>(upd[i]);
      }
      upd = cUppers.top();
      cUppers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolUpper[ind] = get<1>(upd[i]);
      }
      upd = costs.top();
      costs.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolCost[ind] = get<1>(upd[i]);
      }
      break;
    case 2:  // each variable at forcing row: rowDual is cost here
      RcolCost[col] = dualR;
      break;
    case 22:  //
      upd = rLowers.top();
      rLowers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RrowLower[ind] = get<1>(upd[i]);
      }
      upd = rUppers.top();
      rUppers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RrowUpper[ind] = get<1>(upd[i]);
      }
      break;
    case 3:  // the row that is forcing
      if (valC != 0) {
        upd = rLowers.top();
        rLowers.pop();
        for (size_t i = 0; i < upd.size(); i++) {
          HighsInt ind = get<0>(upd[i]);
          RrowLower[ind] = get<1>(upd[i]);
        }
        upd = rUppers.top();
        rUppers.pop();
        for (size_t i = 0; i < upd.size(); i++) {
          HighsInt ind = get<0>(upd[i]);
          RrowUpper[ind] = get<1>(upd[i]);
        }
      }
      break;
    case 4:  // implied free column singleton (also from duplicate row)
      upd = costs.top();
      costs.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolCost[ind] = get<1>(upd[i]);
      }
      break;
    case 5:  // doubleton eq with singleton col
      upd = cLowers.top();
      cLowers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolLower[ind] = get<1>(upd[i]);
      }
      upd = cUppers.top();
      cUppers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolUpper[ind] = get<1>(upd[i]);
      }
      upd = costs.top();
      costs.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolCost[ind] = get<1>(upd[i]);
      }
      break;
    case 17: {  // doubleton equation
      upd = cLowers.top();
      cLowers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolLower[ind] = get<1>(upd[i]);
      }
      upd = cUppers.top();
      cUppers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolUpper[ind] = get<1>(upd[i]);
      }
      upd = costs.top();
      costs.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolCost[ind] = get<1>(upd[i]);
      }
      break;
    }
    case 6:  // empty column, dominated column or weakly dominated
      if (valC != 0) {
        upd = rLowers.top();
        rLowers.pop();
        for (size_t i = 0; i < upd.size(); i++) {
          HighsInt ind = get<0>(upd[i]);
          RrowLower[ind] = get<1>(upd[i]);
        }
        upd = rUppers.top();
        rUppers.pop();
        for (size_t i = 0; i < upd.size(); i++) {
          HighsInt ind = get<0>(upd[i]);
          RrowUpper[ind] = get<1>(upd[i]);
        }
      }
      break;
    case 7:  // fixed variable
      if (valC != 0) {
        upd = rLowers.top();
        rLowers.pop();
        for (size_t i = 0; i < upd.size(); i++) {
          HighsInt ind = get<0>(upd[i]);
          RrowLower[ind] = get<1>(upd[i]);
        }
        upd = rUppers.top();
        rUppers.pop();
        for (size_t i = 0; i < upd.size(); i++) {
          HighsInt ind = get<0>(upd[i]);
          RrowUpper[ind] = get<1>(upd[i]);
        }
      }
      break;
    case 11:  // empty row from duplicate rows
      upd = rLowers.top();
      rLowers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RrowLower[ind] = get<1>(upd[i]);
      }
      upd = rUppers.top();
      rUppers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RrowUpper[ind] = get<1>(upd[i]);
      }
      break;
    case 12:  // doubleton eq from duplicate rows;
      upd = cLowers.top();
      cLowers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolLower[ind] = get<1>(upd[i]);
      }
      upd = cUppers.top();
      cUppers.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolUpper[ind] = get<1>(upd[i]);
      }
      upd = costs.top();
      costs.pop();
      for (size_t i = 0; i < upd.size(); i++) {
        HighsInt ind = get<0>(upd[i]);
        RcolCost[ind] = get<1>(upd[i]);
      }
      break;
    case 121:  //
      break;   /*
  case 14: //two duplicate columns by one
         colValue[col] = valC;
         colDual[col] = dualC;
         RcolLower = cLowers.top(); cLowers.pop();
         RcolUpper = cUppers.top(); cUppers.pop();
         break;
  case 15: //sing variable on initEq
         flagRow[row] = true;
         rowDual[row] = dualR;
         break;*/
  }
}

dev_kkt_check::State KktChStep::initState(
    const HighsInt numCol_, const HighsInt numRow_,
    const std::vector<HighsInt>& Astart_, const std::vector<HighsInt>& Aend_,
    const std::vector<HighsInt>& Aindex_, const std::vector<double>& Avalue_,
    const std::vector<HighsInt>& ARstart_,
    const std::vector<HighsInt>& ARindex_, const std::vector<double>& ARvalue_,
    const std::vector<HighsInt>& flagCol_,
    const std::vector<HighsInt>& flagRow_, const std::vector<double>& colValue_,
    const std::vector<double>& colDual_, const std::vector<double>& rowValue_,
    const std::vector<double>& rowDual_,
    const std::vector<HighsBasisStatus>& col_status_,
    const std::vector<HighsBasisStatus>& row_status_) {
  // check row value

  std::vector<double> rowValue(numRow_, 0);
  for (int row = 0; row < numRow_; row++) {
    if (flagRow_[row]) {
      for (int k = ARstart_[row]; k < ARstart_[row + 1]; k++) {
        const int col = ARindex_[k];
        if (flagCol_[col]) rowValue[row] += ARvalue_[k] * colValue_[col];
      }
      assert(rowValue_[row] == rowValue[row]);
    }
  }

  return dev_kkt_check::State(numCol_, numRow_, Astart_, Aend_, Aindex_,
                              Avalue_, ARstart_, ARindex_, ARvalue_, RcolCost,
                              RcolLower, RcolUpper, RrowLower, RrowUpper,
                              flagCol_, flagRow_, colValue_, colDual_,
                              rowValue_, rowDual_, col_status_, row_status_);
}

}  // namespace dev_kkt_check

}  // namespace presolve
