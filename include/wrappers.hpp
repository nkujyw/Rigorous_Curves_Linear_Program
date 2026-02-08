#pragma once

#include <unordered_map>
#include <vector>

#include "distribution.hpp"

double best_LB(dist_t&, int64_t, double);
std::vector<double> best_LB(dist_t&, std::vector<int64_t>, double);
double best_UB(dist_t&, int64_t, double);
std::vector<double> best_UB(dist_t&, std::vector<int64_t>, double);

std::unordered_map<std::string, double> bound(dist_t&, int64_t, double);
std::unordered_map<std::string, std::vector<double>> bound(dist_t&, std::vector<int64_t>, double);
std::unordered_map<std::string, double> bound(dist_t&, int64_t, double, std::vector<std::string>);
std::unordered_map<std::string, std::vector<double>> bound(dist_t&, std::vector<int64_t>, double, std::vector<std::string>);

