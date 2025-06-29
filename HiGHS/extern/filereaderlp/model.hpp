#ifndef __READERLP_MODEL_HPP__
#define __READERLP_MODEL_HPP__

#include <limits>
#include <memory>
#include <string>
#include <vector>

enum class VariableType {
  CONTINUOUS,
  BINARY,
  GENERAL,
  SEMICONTINUOUS,
  SEMIINTEGER
};

enum class ObjectiveSense { MIN, MAX };

struct Variable {
  VariableType type = VariableType::CONTINUOUS;
  double lowerbound = 0.0;
  double upperbound = std::numeric_limits<double>::infinity();
  std::string name;

  Variable(std::string n = "") : name(n){};
};

struct LinTerm {
  std::shared_ptr<Variable> var;
  double coef = 1.0;
};

struct QuadTerm {
  std::shared_ptr<Variable> var1;
  std::shared_ptr<Variable> var2;
  double coef = 1.0;
};

struct Expression {
  std::vector<std::shared_ptr<LinTerm>> linterms;
  std::vector<std::shared_ptr<QuadTerm>> quadterms;
  double offset = 0.0;
  std::string name = "";
};

struct Constraint {
  double lowerbound = -std::numeric_limits<double>::infinity();
  double upperbound = std::numeric_limits<double>::infinity();
  std::shared_ptr<Expression> expr;

  Constraint() : expr(std::shared_ptr<Expression>(new Expression)){};
};

struct SOS {
  std::string name = "";
  short type = 0;  // 1 or 2
  std::vector<std::pair<std::shared_ptr<Variable>, double>> entries;
};

struct Model {
  std::shared_ptr<Expression> objective;
  ObjectiveSense sense;
  std::vector<std::shared_ptr<Constraint>> constraints;
  std::vector<std::shared_ptr<Variable>> variables;
  std::vector<std::shared_ptr<SOS>> soss;
};

#endif
