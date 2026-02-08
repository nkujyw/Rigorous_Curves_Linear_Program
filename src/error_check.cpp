#include "error_check.hpp"

#include <iostream>

bool error_check_basic(dist_t& dist, int64_t G, double err) {
  if (dist.N == 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: dist_t object is empty.]" << std::endl;
    }
    return false;
  }
  if (G <= 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: G must be a positive interger.]" << std::endl;
    }
    return false;
  }
  if (err <= 0 || err >= 1) {
    if (dist.verbose) {
      std::cerr << "\n[Error: err must be between 0 and 1.]" << std::endl;
    }
    return false;
  }

  return true;
}

bool error_check_basic(dist_t& dist, std::vector<int64_t> Gs, double err) {
  if (dist.N == 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: dist_t object is empty.]" << std::endl;
    }
    return false;
  }
  for (auto G:Gs) {
    if (G <= 0) {
      if (dist.verbose) {
        std::cerr << "\n[Error: All Gs must be positive intergers.]" << std::endl;
      }
      return false;
    }
  }
  if (err <= 0 || err >= 1) {
    if (dist.verbose) {
      std::cerr << "\n[Error: err must be between 0 and 1.]" << std::endl;
    }
    return false;
  }

  return true;
}

bool error_check_with_partition(dist_t& dist, int64_t G, double err) {
  if (!error_check_basic(dist, G, err)) {
    return false;
  }
  if (dist.D2_idx.empty()) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Must partition before calculating sampling LB.]" << std::endl;
    }
    return false;
  }

  return true;
}

bool error_check_with_attack(dist_t& dist, int64_t G, double err) {
  if (!error_check_basic(dist, G, err)) {
    return false;
  }
  if (dist.D2_idx.empty()) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Must partition and attack before calculating extended LB.]" << std::endl;
    }
    return false;
  }
  if (dist.model_attack_filename.empty()) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Must specify attack from model before calculating extended LB.]" << std::endl;
    }
    return false;
  }

  return true;
}

bool error_check_prior_LB(dist_t& dist, int64_t G, int64_t j, double err1, double err2) {
  if (dist.N == 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: dist_t object is empty.]" << std::endl;
    }
    return false;
  }
  if (G <= 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: G must be a positive interger.]" << std::endl;
    }
    return false;
  }
  if (err1 <= 0 || err1 >= 1) {
    if (dist.verbose) {
      std::cerr << "\n[Error: err1 must be between 0 and 1.]" << std::endl;
    }
    return false;
  }
  if (err2 <= 0 || err2 >= 1) {
    if (dist.verbose) {
      std::cerr << "\n[Error: err2 must be between 0 and 1.]" << std::endl;
    }
    return false;
  }
  if (G <= dist.N) {
    if (dist.verbose) {
      std::cerr << "\n[Error: No L value satisfy the constraints on the parameters.]" << std::endl;
    }
    return false;
  }
  if (j < 2) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Invalid j value. j must be greater than or equal to 2.]" << std::endl;
    }
    return false;
  }
  if (G <= dist.N) {
    if (dist.verbose) {
      std::cerr << "\n[Error: No L value satisfy the constraints on the parameters.]" << std::endl;
    }
    return false;
  }

  return true;
}

bool error_check_LP(dist_t& dist, int64_t G, double q, int64_t iprime, std::vector<double> errs, std::vector<double> xhats) {
  if (dist.N == 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: dist_t object is empty.]" << std::endl;
    }
    return false;
  }
  if (G <= 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: G must be a positive interger.]" << std::endl;
    }
    return false;
  }
  if (q <= 1) {
    if (dist.verbose) {
      std::cerr << "\n[Error: q must be greater than 1.]" << std::endl;
    }
    return false;
  }
  if (errs.size() != iprime + 1) {
    if (dist.verbose) {
      std::cerr << "\n[Error: errs must be of size iprime+1.]" << std::endl;
    }
    return false;
  }
  if (xhats.size() != iprime + 1) {
    if (dist.verbose) {
      std::cerr << "\n[Error: xhats must be of size iprime+1.]" << std::endl;
    }
    return false;
  }

  return true;
}

