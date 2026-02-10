#include "lp_bounds.hpp"

#include <iostream>
#include <unordered_map>
#include <cmath>
#include <numeric>

#include "gurobi_c++.h"

#include "helpers.hpp"
#include "error_check.hpp"
/*
LP_UB选定fast, normal, slow三种精度模式
fast: q=1.008 normal: q=1.004 slow: q=1.002
每个模式中调用LP_UB（6个参数）和LP_LB函数计算上界和下界
LP_UB和LP_LB函数中调用LP_upper和LP_lower函数进行线性规划求解
*/

/*
@ parameters:
  dist表F^S，G为猜测次数
  mesh对应X_l表网络，q为网络精度参数
  iprime对应i'，用于区分高频和低频元素，idx为攻击者在网格上的猜测范围
*/
double LP_lower(dist_t& dist, int64_t G, std::vector<double>& mesh, double q, int64_t iprime, int64_t idx, std::vector<double>& eps2s, std::vector<double>& eps3s, std::vector<double>& xhats) {

  int64_t l = mesh.size();
  int64_t N = dist.N;

  std::unordered_map<int64_t, double> good_turing_estimates;
  for (auto x:dist.freqcount) {
    good_turing_estimates[x.first] = ((double) x.first * x.second) / (N - x.first + 1.0);
  }

  try {
    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model = GRBModel(env);

    // variables
    std::vector<GRBVar> hx_vars(l); // hx_vars[i] is h_j * x_j
    for (int i=0; i<l; ++i) {
      hx_vars[i] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "hx_vars["+std::to_string(i)+"]");
    }
    GRBVar p_var = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "p_var");
    GRBVar c_var = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "c_var"); // c_var is c * x_idx

    // objective
    GRBLinExpr obj = 0.0;
    for (int i=0; i<idx-1; ++i) { // idx-1 is because of 1-indexing in paper vs. 0-indexing in program
      obj += hx_vars[i];
    }
    if (idx <= l) {
      obj += c_var;
    }
    model.setObjective(obj, GRB_MINIMIZE);

    // constraints

    // constraint (1)
    GRBLinExpr constr_1_lhs = 0.0;
    double scale = 0.0;
    if(idx<=l){
      scale=(G/mesh[idx-1]>=1.0) ? 1.0/G : mesh[idx-1];
    }
    else{
      scale=1.0/G;
    }
    for (int i=0; i<idx-1; ++i) {
      constr_1_lhs += hx_vars[i] * scale / mesh[i];
    }
    if (idx <= l) {
      constr_1_lhs += c_var * scale / mesh[idx-1];
    }else {
      constr_1_lhs += c_var;
    }
    GRBLinExpr constr_1_rhs = G * scale;
    model.addConstr(constr_1_lhs, GRB_EQUAL, constr_1_rhs, "1) (sum{j<idx} h_j) + c == G");

    // constraint (2)
    for (int i=0; i<=iprime; ++i) {
      GRBLinExpr sum_hxvars_bpdf = 0.0;
      GRBLinExpr lb = 0.0;
      GRBLinExpr ub = 0.0;
      for (int j=0; j<l; ++j) {
        sum_hxvars_bpdf += hx_vars[j] * bpdf(i, N, mesh[j]);
      }
      if (i==0) {
        lb = (1.0 / pow(q, i+1)) * (good_turing_estimates[i+1] - eps2s[i] - ((double) (i+1))/((double) (N-i)) - p_var);
        ub = (1.0 + eps3s[i]) * (good_turing_estimates[i+1] + eps2s[i] - p_var * bpdf(i, N, q * mesh[l-1])) + bpdf(i, N, xhats[i]);
      }
      else {
        lb = (1.0 / pow(q, i+1)) * (good_turing_estimates[i+1] - eps2s[i] - ((double) (i+1))/((double) (N-i)) - p_var * bpdf(i, N, q * mesh[l-1]));
        ub = (1.0 + eps3s[i]) * (good_turing_estimates[i+1] + eps2s[i]) + bpdf(0, N, xhats[i]);
      }
      model.addConstr(lb, GRB_LESS_EQUAL, sum_hxvars_bpdf, "2) lb");
      model.addConstr(sum_hxvars_bpdf, GRB_LESS_EQUAL, ub, "2) ub");
    }
    
    // constraint (3)
    GRBLinExpr left = (1.0 - p_var) / q;
    GRBLinExpr right = 1.0 - p_var;
    GRBLinExpr sum_hxvars = 0.0;
    for (int i=0; i<l; ++i) {
      sum_hxvars += hx_vars[i];
    }
    model.addConstr(left, GRB_LESS_EQUAL, sum_hxvars, "3) 1-p/q <= sum h_j");
    model.addConstr(sum_hxvars, GRB_LESS_EQUAL, right, "3) sum h_j <= 1-p");
    
    // constraint (4)
    if (idx <= l) {
      model.addConstr(c_var <= hx_vars[idx-1], "4) c <= h_idx");
    }


    // optimize 
    model.optimize();

    // status/solution
    int status = model.get(GRB_IntAttr_Status);
		if (status == GRB_OPTIMAL) {
      return model.get(GRB_DoubleAttr_ObjVal);
		}
    else if (status == GRB_INFEASIBLE) {
      return -2;
    }
    else {
      return -1;
    }
  } catch(GRBException e) {
    if (dist.verbose) {
      std::cerr << "\n[Error: code = " << e.getErrorCode() << "; message: " << e.getMessage() << ".]" << std::endl;
    }
    return -3;
  } catch(...) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Exception during optimization.]" << std::endl;
    }
    return -4;
  }

  return 0.0;
}

double LP_upper(dist_t& dist, int64_t G, std::vector<double>& mesh, double q, int64_t iprime, int64_t idx, std::vector<double>& eps2s, std::vector<double>& eps3s, std::vector<double>& xhats) {

  int64_t l = mesh.size();
  int64_t N = dist.N;

  std::unordered_map<int64_t, double> good_turing_estimates;
  for (auto x:dist.freqcount) {
    good_turing_estimates[x.first] = ((double) x.first * x.second) / (N - x.first + 1.0);
  }

  try {
    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();

    GRBModel model = GRBModel(env);

    // variables
    std::vector<GRBVar> hx_vars(l); // hx_vars[i] is h_j * x_j
    for (int i=0; i<l; ++i) {
      hx_vars[i] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "hx_vars["+std::to_string(i)+"]");
    }
    GRBVar p_var = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "p_var");
    GRBVar c_var = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "c_var"); // c_var is c * x_idx

    // objective
    GRBLinExpr obj = 0.0;
    for (int i=0; i<idx-1; ++i) { // idx-1 is because of 1-indexing in paper vs. 0-indexing in program
      obj += hx_vars[i];
    }
    obj += c_var;
    model.setObjective(obj, GRB_MAXIMIZE);

    // constraints

    // constraint (1)
    GRBLinExpr constr_1_lhs = 0.0;
    double scale = 0.0;
    if (idx <= l) {
    scale = (G/mesh[idx-1]>=1.0) ? 1.0/G : mesh[idx-1];
    } else {
    scale = (G/mesh[l-1]>=1.0) ? 1.0/G : mesh[l-1];
    }
    for (int i=0; i<idx-1; ++i) {
      constr_1_lhs += hx_vars[i] * scale / mesh[i];
    }
    if (idx <= l) {
      constr_1_lhs += c_var * scale / mesh[idx-1];
    }
    else {
      constr_1_lhs += c_var * scale / mesh[l-1];
    }
    GRBLinExpr constr_1_rhs = G * scale;
    model.addConstr(constr_1_lhs, GRB_EQUAL, constr_1_rhs, "1) (sum{j<idx} h_j) + c == G");

    // constraint (2)
    for (int i=0; i<=iprime; ++i) {
      GRBLinExpr sum_hxvars_bpdf = 0.0;
      GRBLinExpr lb = 0.0;
      GRBLinExpr ub = 0.0;
      for (int j=0; j<l; ++j) {
        sum_hxvars_bpdf += hx_vars[j] * bpdf(i, N, mesh[j]);
      }
      if (i==0) {
        lb = (1.0 / (1.0 + eps3s[i])) * (good_turing_estimates[i+1] - eps2s[i] - ((double) (i+1))/((double) (N-i)) - p_var - bpdf(i, N, q * xhats[i]));
        ub = pow(q, i+1) * (good_turing_estimates[i+1] + eps2s[i] - p_var * bpdf(i, N, q * mesh[l-1]));
      }
      else {
        lb = (1.0 / (1.0 + eps3s[i])) * (good_turing_estimates[i+1] - eps2s[i] - ((double) (i+1))/((double) (N-i)) - p_var * bpdf(i, N, q * mesh[l-1]) - bpdf(i, N, q * xhats[i]));
        ub = pow(q, i+1) * (good_turing_estimates[i+1] + eps2s[i]);
      }
      model.addConstr(lb, GRB_LESS_EQUAL, sum_hxvars_bpdf);
      model.addConstr(sum_hxvars_bpdf, GRB_LESS_EQUAL, ub);
    }
    
    // constraint (3)
    GRBLinExpr left = 1.0 - p_var;
    GRBLinExpr right = (1.0 - p_var) * q;
    GRBLinExpr sum_hxvars = 0.0;
    for (int i=0; i<l; ++i) {
      sum_hxvars += hx_vars[i];
    }
    model.addConstr(left, GRB_LESS_EQUAL, sum_hxvars, "3) 1-p <= sum h_j");
    model.addConstr(sum_hxvars, GRB_LESS_EQUAL, right, "3) sum h_j <= q*(1-p)");
    
    // constraint (4)
    if (idx <= l) {
      model.addConstr(c_var <= hx_vars[idx-1], "4) c <= h_idx");
    }

    // optimize 
    model.optimize();

    // status/solution
    int status = model.get(GRB_IntAttr_Status);
		if (status == GRB_OPTIMAL) {
      return model.get(GRB_DoubleAttr_ObjVal);
		}
    else if (status == GRB_INFEASIBLE) {
      return -2;
    }
    else {
      return -1;
    }
  } catch(GRBException e) {
    if (dist.verbose) {
      std::cerr << "\n[Error: code = " << e.getErrorCode() << "; message: " << e.getMessage() << ".]" << std::endl;
    }
    return -3;
  } catch(...) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Exception during optimization.]" << std::endl;
    }
    return -4;
  }

  return 1.0;
}

double LP_LB(dist_t& dist, int64_t G, double q, int64_t iprime, std::vector<double> errs, std::vector<double> xhats) {
  // Note: error rate will be 2 * sum(errs)
  if (!error_check_LP(dist, G, q, iprime, errs, xhats)) {
    return -1;
  }

  int64_t N = dist.N;
  std::vector<double> eps2s(iprime+1);
  std::vector<double> eps3s(iprime+1);
  for (int i=0; i<=iprime; ++i) {
    eps2s[i] = sqrt(-N * log(errs[i]) / 2.0) * ((((double) i) + 1.0) / ((double) (N - i)));
    double log_eps3 = (N - i) * log((1 - xhats[i]) / (1 - q*xhats[i])) - (i + 1) * log(q);
    eps3s[i] = exp(log_eps3) - 1;
  }

  int64_t l = ((int64_t) floor((log(10000.0) + log((double) N)) / log(q))) + 1;
  std::vector<double> mesh(l);
  mesh[l-1] = 1.0 / (10000.0 * N);
  for (int i=l-2; i>=0; --i) {
    mesh[i] = mesh[i+1] * q;
  }

  double res = 1.0;
  double lp_bound;
  bool feasible = false;
  for (int64_t idx=1; idx<=l+1; ++idx) {
    lp_bound = LP_lower(dist, G, mesh, q, iprime, idx, eps2s, eps3s, xhats);
    if (lp_bound > 0) {
      feasible = true;
      res = std::min(res, lp_bound);
    }
  }
  if (!feasible) {
    if (dist.verbose) {
      std::cerr << "\n[Model is infeasible! Sample might not be drawn iid from the underlying distribution.]" << std::endl;
    }
    return -2;
  }
  return std::max(res, 0.0);
}

double LP_UB(dist_t& dist, int64_t G, double q, int64_t iprime, std::vector<double> errs, std::vector<double> xhats) {
  // Note: error rate will be 2 * sum(errs)
  if (!error_check_LP(dist, G, q, iprime, errs, xhats)) {
    return -1;
  }

  int64_t N = dist.N;

  std::vector<double> eps2s(iprime+1);
  std::vector<double> eps3s(iprime+1);
  for (int i=0; i<=iprime; ++i) {
    eps2s[i] = sqrt(-N * log(errs[i]) / 2.0) * ((((double) i) + 1.0) / ((double) (N - i)));
    double log_eps3 = (N - i) * log((1 - xhats[i]) / (1 - q*xhats[i])) - (i + 1) * log(q);
    eps3s[i] = exp(log_eps3) - 1;
  }

  int64_t l = ((int64_t) floor((log(10000.0) + log((double) N)) / log(q))) + 1;
  std::vector<double> mesh(l);
  mesh[l-1] = 1.0 / (10000.0 * N);
  for (int i=l-2; i>=0; --i) {
    mesh[i] = mesh[i+1] * q;
  }

  double res = 0.0;
  double lp_bound;
  bool feasible = false;
  for (int64_t idx=1; idx<=l+1; ++idx) {
    lp_bound = LP_upper(dist, G, mesh, q, iprime, idx, eps2s, eps3s, xhats);
    if (lp_bound > 0) {
      feasible = true;
      res = std::max(res, lp_bound);
    }
  }
  if (!feasible) {
    if (dist.verbose) {
      std::cerr << "\n[Model is infeasible! Sample might not be drawn iid from the underlying distribution.]" << std::endl;
    }
    return -2;
  }
  return std::min(res, 1.0);
}

double LP_LB(dist_t& dist, int64_t G, double err) {
  return LP_LB_normal(dist, G, err);
}

double LP_UB(dist_t& dist, int64_t G, double err) {
  return LP_UB_normal(dist, G, err);
}

// Fast version with q=1.008,iprime=6
double LP_LB_fast(dist_t& dist, int64_t G, double err) {
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }
  double q = 1.008;
  int64_t iprime = 6;
  std::vector<double> xhats = {3.0/dist.N, 4.0/dist.N, 5.0/dist.N, 6.0/dist.N, 7.0/dist.N, 8.0/dist.N, 9.0/dist.N};
  std::vector<double> errs = {1, 1.5, 1.75, 1.875, 1.9375, 1.96875, 1.984375};
  double sum = std::accumulate(errs.begin(), errs.end(), 0.0);
  for (auto& x:errs) {
    x *= (err / (2.0 * sum));
  }
  return LP_LB(dist, G, q, iprime, errs, xhats);
}

double LP_UB_fast(dist_t& dist, int64_t G, double err) {
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }
  double q = 1.008;
  int64_t iprime = 6;
  std::vector<double> xhats = {3.0/dist.N, 4.0/dist.N, 5.0/dist.N, 6.0/dist.N, 7.0/dist.N, 8.0/dist.N, 9.0/dist.N};
  std::vector<double> errs = {1, 1.5, 1.75, 1.875, 1.9375, 1.96875, 1.984375};
  double sum = std::accumulate(errs.begin(), errs.end(), 0.0);
  for (auto& x:errs) {
    x *= (err / (2.0 * sum));
  }
  return LP_UB(dist, G, q, iprime, errs, xhats);
}

// Normal version with q=1.004, iprime=6
double LP_LB_normal(dist_t& dist, int64_t G, double err) {
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }

  double q = 1.004;
  int64_t iprime = 6;
  std::vector<double> xhats = {3.0/dist.N, 4.0/dist.N, 5.0/dist.N, 6.0/dist.N, 7.0/dist.N, 8.0/dist.N, 9.0/dist.N};
  std::vector<double> errs = {1, 1.5, 1.75, 1.875, 1.9375, 1.96875, 1.984375};
  double sum = std::accumulate(errs.begin(), errs.end(), 0.0);
  for (auto& x:errs) {
    x *= (err / (2.0 * sum));
  }
  return LP_LB(dist, G, q, iprime, errs, xhats);
}

double LP_UB_normal(dist_t& dist, int64_t G, double err) {
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }

  double q = 1.004;
  int64_t iprime = 6;
  std::vector<double> xhats = {3.0/dist.N, 4.0/dist.N, 5.0/dist.N, 6.0/dist.N, 7.0/dist.N, 8.0/dist.N, 9.0/dist.N};
  std::vector<double> errs = {1, 1.5, 1.75, 1.875, 1.9375, 1.96875, 1.984375};
  double sum = std::accumulate(errs.begin(), errs.end(), 0.0);
  for (auto& x:errs) {
    x *= (err / (2.0 * sum));
  }
  return LP_UB(dist, G, q, iprime, errs, xhats);
}

// Slow version with q=1.002,iprime=4,err=0.01置信度为99%
double LP_LB_slow(dist_t& dist, int64_t G, double err=0.01) {
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }

  double q = 1.002;
  int64_t iprime = 4;
  std::vector<double> xhats = {7.0/dist.N, 11.0/dist.N, 14.0/dist.N, 16.3/dist.N, 18.5/dist.N};
  std::vector<double> errs = {0.00009, 0.000165, 0.00175, 0.00175, 0.0012};
  // double sum = std::accumulate(errs.begin(), errs.end(), 0.0);
  // for (auto& x:errs) {
  //   x *= (err / (2.0 * sum));
  // }
  return LP_LB(dist, G, q, iprime, errs, xhats);
}

double LP_UB_slow(dist_t& dist, int64_t G, double err=0.01) {
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }

  double q = 1.002;
  int64_t iprime = 4;
  std::vector<double> xhats = {7.0/dist.N, 11.0/dist.N, 14.0/dist.N, 16.3/dist.N, 18.5/dist.N};
  std::vector<double> errs = {0.00009, 0.000165, 0.00175, 0.00175, 0.0012};
  // double sum = std::accumulate(errs.begin(), errs.end(), 0.0);
  // for (auto& x:errs) {
  //   x *= (err / (2.0 * sum));
  // }
  return LP_UB(dist, G, q, iprime, errs, xhats);
}

