#pragma once

#include <vector>

#include "distribution.hpp"

bool error_check_basic(dist_t&, int64_t, double);
bool error_check_basic(dist_t&, std::vector<int64_t>, double);
bool error_check_with_partition(dist_t&, int64_t, double);
bool error_check_with_attack(dist_t&, int64_t, double);
bool error_check_prior_LB(dist_t&, int64_t, int64_t, double, double);
bool error_check_LP(dist_t&, int64_t, double, int64_t, std::vector<double>, std::vector<double>);

