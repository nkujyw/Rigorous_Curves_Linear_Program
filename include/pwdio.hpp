#pragma once

#include <vector>

#include "distribution.hpp"

void parse_freqcount(dist_t&, std::vector<std::pair<int64_t, int64_t>>&);
bool read_plain(dist_t&, std::string);
bool read_pwdfreq(dist_t&, std::string); // pwd freq seperated with \t
bool read_freqcount(dist_t&, std::string);
// other formats ??
bool write_freqcount(dist_t&, std::string);
bool read_file(dist_t&, std::string, std::string);
