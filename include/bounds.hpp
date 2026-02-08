#pragma once

#include <stdint.h>

#include "distribution.hpp"

// LP paper
double freq_UB(dist_t&, int64_t, double); // Coro 4
double samp_LB(dist_t&, int64_t, double); // Thm 5
double extended_LB(dist_t&, int64_t, double); // Coro 7

double prior_LB(dist_t&, int64_t, int64_t, double, double); // Thm 9
double prior_LB(dist_t&, int64_t, int64_t, double); // Thm 9
double best_prior_LB(dist_t&, int64_t, double, double); // Thm 9
double best_prior_LB(dist_t&, int64_t, double); // Thm 9, automatically selects parameters

// PIN paper
double binom_LB(dist_t&, int64_t, double); // Coro 4
double binom_UB(dist_t&, int64_t, double); // Thm 2
