#include "HCheckConfig.h"
#include "Highs.h"
#include "catch.hpp"
#include "lp_data/HConst.h"

const double inf = kHighsInf;
const bool dev_run = false;
const double double_equal_tolerance = 1e-5;
const HighsVarType continuous = HighsVarType::kContinuous;
const HighsVarType semi_continuous = HighsVarType::kSemiContinuous;
const HighsVarType semi_integer = HighsVarType::kSemiInteger;

void semiModel0(HighsLp& lp);

TEST_CASE("semi-variable-model", "[highs_test_semi_variables]") {
  Highs highs;
  const HighsInfo& info = highs.getInfo();
  HighsStatus return_status;
  double optimal_objective_function_value;
  if (!dev_run) highs.setOptionValue("output_flag", false);
  HighsModel model;
  HighsLp& lp = model.lp_;
  semiModel0(lp);
  const HighsInt semi_col = 2;
  const double semi_col_cost = -4.0;
  const double semi_col_lower = lp.col_lower_[semi_col];
  const double semi_col_upper = lp.col_upper_[semi_col];
  lp.col_cost_[semi_col] = semi_col_cost;
  optimal_objective_function_value = 6.83333;
  // Legal to have infinte upper bounds on semi-variables
  lp.col_upper_[semi_col] = inf;
  return_status = highs.passModel(model);
  REQUIRE(return_status == HighsStatus::kOk);

  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(!highs.getLp().hasMods());
  REQUIRE(highs.getModelStatus() == HighsModelStatus::kOptimal);
  if (dev_run) highs.writeSolution("", kSolutionStylePretty);
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  // Remove the semi-condition and resolve - not the same as relaxation
  highs.changeColIntegrality(semi_col, continuous);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(!highs.getLp().hasMods());
  if (dev_run) highs.writeSolution("", kSolutionStylePretty);
  optimal_objective_function_value = 3.93333;
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  // Restore the semi-condition, change the cost and resolve
  highs.changeColIntegrality(semi_col, semi_continuous);
  highs.changeColCost(semi_col, -0.1);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(!highs.getLp().hasMods());
  if (dev_run) highs.writeSolution("", kSolutionStylePretty);
  optimal_objective_function_value = 8.22333;
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  // Fix the variable at zero and resolve
  highs.changeColBounds(semi_col, 0, 0);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(!highs.getLp().hasMods());
  if (dev_run) highs.writeSolution("", kSolutionStylePretty);
  optimal_objective_function_value = 6.83333;
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  // Change to semi-integer, restore the bounds and resolve
  highs.changeColIntegrality(semi_col, semi_integer);
  highs.changeColBounds(semi_col, semi_col_lower, semi_col_upper);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(!highs.getLp().hasMods());
  if (dev_run) highs.writeSolution("", kSolutionStylePretty);
  optimal_objective_function_value = 8.13333;
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  // Ensure that a user solution is handled properly
  HighsSolution sol;
  sol.col_value = {0, 0, 0.5, 0};
  highs.setSolution(sol);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(!highs.getLp().hasMods());
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  highs.resetGlobalScheduler(true);
}

TEST_CASE("semi-variable-lower-bound", "[highs_test_semi_variables]") {
  const double optimal_relaxation_objective_function_value = 7.83333;
  const double optimal_semi_continuous_objective_function_value = 7.23333;
  double optimal_objective_function_value;
  Highs highs;
  const HighsInfo& info = highs.getInfo();
  highs.setOptionValue("output_flag", dev_run);
  HighsLp lp;
  semiModel0(lp);
  const HighsInt semi_col = 2;
  const double semi_col_cost = -1.0;
  const double semi_col_lower = lp.col_lower_[semi_col];
  lp.col_cost_[semi_col] = semi_col_cost;
  // Force relaxation directly
  lp.col_lower_[semi_col] = 0;
  lp.integrality_[semi_col] = continuous;
  REQUIRE(highs.passModel(lp) == HighsStatus::kOk);

  REQUIRE(highs.run() == HighsStatus::kOk);
  if (dev_run) highs.writeSolution("", kSolutionStylePretty);

  optimal_objective_function_value =
      optimal_relaxation_objective_function_value;
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  // Restore the semi-continuous variable
  lp.col_lower_[semi_col] = semi_col_lower;
  lp.integrality_[semi_col] = semi_continuous;
  REQUIRE(highs.passModel(lp) == HighsStatus::kOk);

  REQUIRE(highs.run() == HighsStatus::kOk);
  if (dev_run) highs.writeSolution("", kSolutionStylePretty);
  optimal_objective_function_value =
      optimal_semi_continuous_objective_function_value;
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  // Now solve the relaxation
  highs.setOptionValue("solve_relaxation", true);
  REQUIRE(highs.run() == HighsStatus::kOk);
  if (dev_run) highs.writeSolution("", kSolutionStylePretty);
  optimal_objective_function_value =
      optimal_relaxation_objective_function_value;
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);
  // Check that the lower bound of the semi-variable has been restored
  REQUIRE(highs.getLp().col_lower_[semi_col] == semi_col_lower);

  highs.resetGlobalScheduler(true);
}

TEST_CASE("semi-variable-upper-bound", "[highs_test_semi_variables]") {
  Highs highs;
  if (!dev_run) highs.setOptionValue("output_flag", false);
  HighsLp lp;
  lp.num_col_ = 2;
  lp.num_row_ = 0;
  lp.col_cost_ = {1, 2};
  lp.col_lower_ = {1, 0};
  lp.col_upper_ = {inf, 1};
  lp.sense_ = ObjSense::kMaximize;
  lp.integrality_ = {HighsVarType::kSemiContinuous, HighsVarType::kContinuous};

  REQUIRE(highs.passModel(lp) == HighsStatus::kOk);

  // Problem is unbounded due to infinite upper bound on x0, so
  // modified upper bound is active in solution, and run returns error
  REQUIRE(highs.run() == HighsStatus::kError);
  REQUIRE(highs.getModelStatus() == HighsModelStatus::kSolveError);

  double lower = kMaxSemiVariableUpper;
  double upper = inf;
  if (dev_run)
    printf("\nModifying the bounds on semi-continuous variable to [%g, %g]\n",
           lower, upper);
  REQUIRE(highs.changeColBounds(0, lower, upper) == HighsStatus::kOk);
  // Problem is still unbounded due to infinite upper bound on x0, but
  // lower bound is too large to set modified upper bound, so run
  // returns error
  REQUIRE(highs.run() == HighsStatus::kError);
  REQUIRE(highs.getModelStatus() == HighsModelStatus::kSolveError);

  lower = 1;
  upper = inf;
  if (dev_run)
    printf("\nModifying the bounds on semi-continuous variable to [%g, %g]\n",
           lower, upper);
  REQUIRE(highs.changeColBounds(0, lower, upper) == HighsStatus::kOk);
  double coeff = 1e6;
  std::vector<HighsInt> index = {0, 1};
  std::vector<double> value = {-1, coeff};
  REQUIRE(highs.addRow(0, 0, 2, index.data(), value.data()) ==
          HighsStatus::kOk);
  // Problem is no longer unbounded due to equation linking the
  // semi-variable to the continuous variable. However, optimal value
  // of semi-variable should be 1e6, so it is active at the modified upper
  // bound.
  REQUIRE(highs.run() == HighsStatus::kError);
  REQUIRE(highs.getModelStatus() == HighsModelStatus::kSolveError);

  HighsInt iRow = 0;
  HighsInt iCol = 1;
  coeff /= 20;
  if (dev_run)
    printf("\nModifying coefficient [%d, %d] to %g\n", (int)iRow, (int)iCol,
           coeff);
  highs.changeCoeff(iRow, iCol, coeff);
  // Problem is no longer unbounded due to equation linking the
  // semi-variable to the continuous variable. However, modified coefficient
  // means that the optimal value of semi-variable is 1e4, so
  // problem is solved OK
  REQUIRE(highs.run() == HighsStatus::kOk);
  if (dev_run) highs.writeSolution("", 1);
  REQUIRE(highs.getModelStatus() == HighsModelStatus::kOptimal);

  highs.resetGlobalScheduler(true);
}

TEST_CASE("semi-variable-file", "[highs_test_semi_variables]") {
  Highs highs;
  const HighsInfo& info = highs.getInfo();
  double optimal_objective_function_value;
  if (!dev_run) highs.setOptionValue("output_flag", false);
  std::string model = "";
  std::string model_file;
  // Solve the same semi-continuous model from MPS and .lp files
  model = "semi-continuous";
  optimal_objective_function_value = 8.22333;
  if (dev_run) printf("\nSolving %s model from MPS file\n", model.c_str());
  model_file = std::string(HIGHS_DIR) + "/check/instances/" + model + ".mps";
  REQUIRE(highs.readModel(model_file) == HighsStatus::kOk);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);
  if (dev_run) printf("\nSolving %s model from LP file\n", model.c_str());
  model_file = std::string(HIGHS_DIR) + "/check/instances/" + model + ".lp";
  REQUIRE(highs.readModel(model_file) == HighsStatus::kOk);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);
  // Solve the same semi-integer model from MPS and .lp files
  model = "semi-integer";
  optimal_objective_function_value = 8.13333;
  if (dev_run) printf("\nSolving %s model from MPS file\n", model.c_str());
  model_file = std::string(HIGHS_DIR) + "/check/instances/" + model + ".mps";
  REQUIRE(highs.readModel(model_file) == HighsStatus::kOk);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);
  if (dev_run) printf("\nSolving %s model from LP file\n", model.c_str());
  model_file = std::string(HIGHS_DIR) + "/check/instances/" + model + ".lp";
  REQUIRE(highs.readModel(model_file) == HighsStatus::kOk);
  REQUIRE(highs.run() == HighsStatus::kOk);
  REQUIRE(fabs(info.objective_function_value -
               optimal_objective_function_value) < double_equal_tolerance);

  highs.resetGlobalScheduler(true);
}

TEST_CASE("semi-variable-inconsistent-bounds", "[highs_test_semi_variables]") {
  HighsLp lp;
  lp.num_col_ = 1;
  lp.num_row_ = 0;
  lp.col_cost_ = {1};
  lp.col_lower_ = {1};
  lp.col_upper_ = {-1};
  lp.a_matrix_.start_ = {0, 0};
  lp.integrality_ = {semi_continuous};
  Highs highs;
  highs.setOptionValue("output_flag", dev_run);
  highs.passModel(lp);
  highs.run();
  REQUIRE(highs.getModelStatus() == HighsModelStatus::kOptimal);
  REQUIRE(highs.getSolution().col_value[0] == 0);
  // Ensure that inconsistent bounds with negative lower are still
  // accepted
  lp.col_lower_[0] = -1;
  lp.col_upper_[0] = -2;
  highs.passModel(lp);
  highs.run();
  REQUIRE(highs.getModelStatus() == HighsModelStatus::kOptimal);
  REQUIRE(highs.getSolution().col_value[0] == 0);
  // Ensure that continuous variables with inconsistent bounds yield
  // infeasibility
  highs.setOptionValue("solve_relaxation", true);
  highs.passModel(lp);
  highs.run();
  REQUIRE(highs.getModelStatus() == HighsModelStatus::kInfeasible);

  highs.resetGlobalScheduler(true);
}

TEST_CASE("semi-variable-inf-upper", "[highs_test_semi_variables]") {
  const std::string test_name = Catch::getResultCapture().getCurrentTestName();
  const std::string test_mps = test_name + ".mps";
  // Introduced due to a semi-variable possibly having an infinite
  // upper bound that needs to be written to MPS in order to define
  // variable type
  Highs highs;
  highs.setOptionValue("output_flag", dev_run);
  HighsModel model;
  HighsLp& lp = model.lp_;
  semiModel0(lp);
  highs.passModel(lp);
  highs.run();
  const double obj0 = highs.getObjectiveValue();
  if (dev_run) printf("Optimum at first run: %g\n", obj0);

  // now write out to MPS and load again
  highs.writeModel(test_mps);
  highs.readModel(test_mps);
  highs.run();
  const double obj1 = highs.getObjectiveValue();
  if (dev_run)
    printf("Optimum at second run (after writing and loading again): %g\n",
           obj1);
  REQUIRE(obj0 == obj1);
  std::remove(test_mps.c_str());

  highs.resetGlobalScheduler(true);
}

void semiModel0(HighsLp& lp) {
  lp.num_col_ = 4;
  lp.num_row_ = 4;
  lp.col_cost_ = {1, 2, -1, -3};
  lp.col_lower_ = {0, 0, 1.1, 0};
  lp.col_upper_ = {inf, inf, inf, inf};
  lp.row_lower_ = {-inf, 0, 0, 0.5};
  lp.row_upper_ = {5, inf, inf, inf};
  lp.a_matrix_.start_ = {0, 3, 6, 7, 8};
  lp.a_matrix_.index_ = {0, 1, 2, 0, 1, 2, 3, 3};
  lp.a_matrix_.value_ = {1, 2, -1, 1, -1, 3, 1, 1};
  lp.a_matrix_.format_ = MatrixFormat::kColwise;
  lp.sense_ = ObjSense::kMaximize;
  lp.integrality_ = {continuous, continuous, semi_continuous, continuous};
}
