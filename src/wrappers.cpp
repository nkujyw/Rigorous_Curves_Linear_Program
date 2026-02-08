#include "wrappers.hpp"

#include <iostream>

#include "bounds.hpp"
#include "lp_bounds.hpp"
#include "error_check.hpp"

double best_LB(dist_t& dist, int64_t G, double err) {
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }

  if (dist.D2_idx.empty()) {
    partition(dist, 0.001);
  }

  double lb = binom_LB(dist, G, err);
  double ub = binom_UB(dist, G, err);
  double threshold = 0.05;

  if (ub - lb > threshold) {
    double lp_lb = LP_LB(dist, G, err);
    if (lp_lb > 0) {
      lb = std::max(lb, lp_lb);
    }
    else if (lp_lb == -2 && dist.verbose) {
      std::cerr << "\n[Warning: LP is infeasible, sample might not be iid.]" << std::endl;
    }
  }

  if (dist.model_attack_hits.size() > 0 && G > dist.N) {
    lb = std::max(lb, extended_LB(dist, G, err));
  }

  return lb;
}

std::vector<double> best_LB(dist_t& dist, std::vector<int64_t> Gs, double err) {
  if (!error_check_basic(dist, Gs, err)) {
    return std::vector<double>();
  }

  std::vector<double> res;
  for (auto G:Gs) {
    res.push_back(best_LB(dist, G, err));
  }

  return res;
}

double best_UB(dist_t& dist, int64_t G, double err) {
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }

  if (dist.D2_idx.empty()) {
    partition(dist, 0.001);
  }

  double lb = binom_LB(dist, G, err);
  double ub = binom_UB(dist, G, err);
  double threshold = 0.05;

  if (ub - lb > threshold) {
    double lp_ub = LP_UB(dist, G, err);
    if (lp_ub > 0) {
      ub = std::min(ub, lp_ub);
    }
    else if (lp_ub == -2 && dist.verbose) {
      std::cerr << "\n[Warning: LP is infeasible, sample might not be iid.]" << std::endl;
    }
  }

  return ub;
}

std::vector<double> best_UB(dist_t& dist, std::vector<int64_t> Gs, double err) {
  if (!error_check_basic(dist, Gs, err)) {
    return std::vector<double>();
  }

  std::vector<double> res;
  for (auto G:Gs) {
    res.push_back(best_UB(dist, G, err));
  }

  return res;
}

std::unordered_map<std::string, double> bound(dist_t& dist, int64_t G, double err) {
  std::unordered_map<std::string, double> res;

  if (!error_check_basic(dist, G, err)) {
    return res;
  }

  if (dist.D2_idx.size() > 0) {
    res["samp LB"] = samp_LB(dist, G, err);
  }
  if (dist.model_attack_hits.size() > 0) {
    res["extended LB"] = extended_LB(dist, G, err);
  }
  res["freq UB"] = freq_UB(dist, G, err);
  res["LP LB"] = LP_LB(dist, G, err);
  res["LP UB"] = LP_UB(dist, G, err);
  res["binom LB"] = binom_LB(dist, G, err);
  res["binom UB"] = binom_UB(dist, G, err);

  return res;
}

std::unordered_map<std::string, std::vector<double>> bound(dist_t& dist, std::vector<int64_t> Gs, double err) {
  std::unordered_map<std::string, std::vector<double>> res;

  if (!error_check_basic(dist, Gs, err)) {
    return res;
  }

  for (auto G:Gs) {
    if (dist.D2_idx.size() > 0) res["samp LB"].push_back(samp_LB(dist, G, err));
    if (dist.model_attack_hits.size() > 0) res["extended LB"].push_back(extended_LB(dist, G, err));
    res["freq UB"].push_back(freq_UB(dist, G, err));
    res["LP LB"].push_back(LP_LB(dist, G, err));
    res["LP UB"].push_back(LP_UB(dist, G, err));
    res["binom LB"].push_back(binom_LB(dist, G, err));
    res["binom UB"].push_back(binom_UB(dist, G, err));
  }

  return res;
}

std::unordered_map<std::string, double> bound(dist_t& dist, int64_t G, double err, std::vector<std::string> bounds) {
  std::unordered_map<std::string, double> res;

  if (!error_check_basic(dist, G, err)) {
    return res;
  }

  auto in_bounds = [&](std::string s) {
    return std::find(bounds.begin(), bounds.end(), s) != bounds.end();
  };

  if (in_bounds("samp LB") && dist.D2_idx.size() > 0) {
    res["samp LB"] = samp_LB(dist, G, err);
  }
  if (in_bounds("extended_LB") && dist.model_attack_hits.size() > 0) {
    res["extended LB"] = extended_LB(dist, G, err);
  }
  if (in_bounds("freq UB")) {
    res["freq UB"] = freq_UB(dist, G, err);
  }
  if (in_bounds("LP LB")) {
    res["LP LB"] = LP_LB(dist, G, err);
  }
  if (in_bounds("LP UB")) {
    res["LP UB"] = LP_UB(dist, G, err);
  }
  if (in_bounds("binom LB")) {
    res["binom LB"] = binom_LB(dist, G, err);
  }
  if (in_bounds("binom UB")) {
    res["binom UB"] = binom_UB(dist, G, err);
  }

  return res;
}

std::unordered_map<std::string, std::vector<double>> bound(dist_t& dist, std::vector<int64_t> Gs, double err, std::vector<std::string> bounds) {
  std::unordered_map<std::string, std::vector<double>> res;

  if (!error_check_basic(dist, Gs, err)) {
    return res;
  }

  auto in_bounds = [&](std::string s) {
    return std::find(bounds.begin(), bounds.end(), s) != bounds.end();
  };

  if (in_bounds("samp LB")) {
    if (dist.D2_idx.empty()) {
      if (dist.verbose) {
        std::cerr << "\n[Error: Must partition before calculating sampling LB.]" << std::endl;
      }
    }
    else {
      for (auto G:Gs) {
        res["samp LB"].push_back(samp_LB(dist, G, err));
      }
    }
  }
  if (in_bounds("extended_LB") && dist.model_attack_hits.size() > 0) {
    if (dist.D2_idx.empty()) {
      if (dist.verbose) {
        std::cerr << "\n[Error: Must partition and attack before calculating extended LB.]" << std::endl;
      }
    }
    else if (dist.model_attack_filename.empty()) {
      if (dist.verbose) {
        std::cerr << "\n[Error: Must specify attack from model before calculating extended LB.]" << std::endl;
      }
    }
    else {
      for (auto G:Gs) {
        res["extended LB"].push_back(extended_LB(dist, G, err));
      }
    }
  }
  if (in_bounds("freq UB")) {
    for (auto G:Gs) {
      res["freq UB"].push_back(freq_UB(dist, G, err));
    }
  }
  if (in_bounds("LP LB")) {
    for (auto G:Gs) {
      res["LP LB"].push_back(LP_LB(dist, G, err));
    }
  }
  if (in_bounds("LP UB")) {
    for (auto G:Gs) {
      res["LP UB"].push_back(LP_UB(dist, G, err));
    }
  }
  if (in_bounds("binom LB")) {
    for (auto G:Gs) {
      res["binom LB"].push_back(binom_LB(dist, G, err));
    }
  }
  if (in_bounds("binom UB")) {
    for (auto G:Gs) {
      res["binom UB"].push_back(binom_UB(dist, G, err));
    }
  }

  return res;
}

