#pragma once


#include <vector>
#include <string>
bool plot(std::string filename);
bool tikz_plot(std::vector<std::vector<std::pair<int64_t, double>>>&, std::vector<std::string>&, std::vector<std::string>&, std::string);
