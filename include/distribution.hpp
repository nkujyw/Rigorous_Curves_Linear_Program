#pragma once

#include <vector>
#include <string>
#include <unordered_map>

struct dist_t {
  std::string filename;
  std::string filetype;

  std::vector<std::pair<int64_t, int64_t>> freqcount;
  std::vector<int64_t> preftotal;
  std::vector<int64_t> prefcount;

  std::vector<int64_t> D2_idx;
  std::unordered_map<std::string, int64_t> D2_hist;
  std::vector<std::pair<int64_t, int64_t>> D1_attack_hits;
  std::string model_attack_filename = "";
  std::vector<std::pair<int64_t, int64_t>> model_attack_hits;

  int64_t N = 0;
  int64_t distinct = 0;
  int64_t distinct_D1 = 0;

  bool verbose = true;
};

void print1(dist_t&);
void print2(dist_t&);
void print3(dist_t&);
void print4(dist_t&);

void set_verbose(dist_t&, bool);

int64_t most_frequent(dist_t&, int64_t);

void partition_small_d(dist_t&, int64_t);
void partition_large_d(dist_t&, int64_t);

bool count_in_partition(dist_t&, std::unordered_map<std::string, int64_t>&, std::unordered_map<std::string, int64_t>&);
bool write_partition(dist_t&, std::unordered_map<std::string, int64_t>&, std::unordered_map<std::string, int64_t>&, std::string, std::string);

bool partition(dist_t&, int64_t, std::string = "", std::string = "");
bool partition(dist_t&, double, std::string = "", std::string = "");

bool pre_partition(dist_t&, int64_t);

void model_attack(dist_t&, std::string);
