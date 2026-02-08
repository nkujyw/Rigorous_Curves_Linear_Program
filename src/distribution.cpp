#include "distribution.hpp"

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>

void print1(dist_t& d) {
  std::cout << "-----------------\n";
  std::cout << "Dataset\n";
  std::cout << "Filename: " << d.filename << "; Filetype: " << d.filetype << '\n';
  std::cout << "Distinct: " << d.distinct << "; Total: " << d.N << '\n';
  std::cout << "freqcount:\n";
  for (auto x:d.freqcount) {
    std::cout << "  (" << x.first << ", " << x.second << ")\n";
  }
  std::cout << "-----------------\n";
}

void print2(dist_t& d) {
  std::cout << "-----------------\n";
  std::cout << "Dataset\n";
  std::cout << "Filename: " << d.filename << "; Filetype: " << d.filetype << '\n';
  std::cout << "Distinct: " << d.distinct << "; Total: " << d.N << '\n';
  std::cout << "Distinct D1: " << d.distinct_D1 << std::endl;
  std::cout << "D1 attack: " << std::endl;
  // for (auto x:d.D1_attack_hits) {
  //   std::cout << x.first << ' ' << x.second << std::endl;
  // }
  for (int i=0; i<15 && i<d.D1_attack_hits.size(); ++i) {
    std::cout << d.D1_attack_hits[i].first << ' ' << d.D1_attack_hits[i].second << std::endl;
  }
  std::cout << "-----------------\n";
}

void set_verbose(dist_t& d, bool verbose) {
  d.verbose = verbose;
}

int64_t most_frequent(dist_t& dist, int64_t G) { // cumulative frequency of top G most frequent passwords
  auto it = std::lower_bound(dist.prefcount.begin(), dist.prefcount.end(), G);
  if (it == dist.prefcount.begin()) {
    return G * dist.freqcount[0].first;
  }
  else if (it == dist.prefcount.end()) {
    return dist.N;
  }
  else {
    int64_t id = it - dist.prefcount.begin();
    return dist.preftotal[id] - (dist.prefcount[id] - G) * dist.freqcount[id].first;
  }
}

void partition_small_d(dist_t& dist, int64_t d) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> uniform_dist(1, dist.N);
  std::unordered_set<int> appeared;

  dist.D2_idx.resize(d);
  for (int i=0; i<d; ++i) {
    while (1) {
      int choice = uniform_dist(gen);
      if (appeared.insert(choice).second) {
        dist.D2_idx[i] = choice;
        break;
      }
    }
  }
}

void partition_large_d(dist_t& dist, int64_t d) {
  std::vector<int64_t> v(dist.N);
  for (int i=0; i<v.size(); ++i) {
    v[i] = i+1;
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(v.begin(), v.end(), gen);
  for (int i=0; i<d; ++i) {
    dist.D2_idx[i] = v[i];
  }
}

bool count_in_partition(dist_t& dist, std::unordered_map<std::string, int64_t>& hist_D1, std::unordered_map<std::string, int64_t>& hist_D2) {
  std::ifstream fin(dist.filename);
  if (!fin.is_open()) {
    if (dist.verbose) {
      std::cerr << "\n[Error: can't open file " << dist.filename << ". Nothing done.]" << std::endl;
    }
    return false;
  }
  std::string pwd;
  std::string line;

  if (dist.filetype == "plain") {
    int64_t i = 0;
    int64_t cnt = 1;
    while (getline(fin, pwd)) {
      if (i <= dist.D2_idx.size() && cnt == dist.D2_idx[i]) {
        hist_D2[pwd]++;
        ++i;
      }
      else {
        hist_D1[pwd]++;
      }
      ++cnt;
    }
  }
  else {
    int64_t freq;
    int64_t cumulative_freq = 0;
    int64_t i = 0;
    while (std::getline(fin, line)) {
      std::istringstream ss(line);
      if (std::getline(ss, pwd, '\t') && ss >> freq) {
        cumulative_freq += freq;
        int64_t in_D2_cnt = 0;
        while (i < dist.D2_idx.size() && dist.D2_idx[i] <= cumulative_freq) {
          ++i;
          ++in_D2_cnt;
        }
        if (freq - in_D2_cnt != 0) hist_D1[pwd] = (freq - in_D2_cnt);
        if (in_D2_cnt != 0) hist_D2[pwd] = in_D2_cnt;
      }
      else {
        if (dist.verbose) {
          std::cerr << "\n[Error: Invalid line: " << line << " in file " << dist.filename << ".]" << std::endl;
        }
      }
    }
  }

  fin.close();

  return true;
}

bool write_partition(dist_t& dist, std::unordered_map<std::string, int64_t>& hist_D1, std::unordered_map<std::string, int64_t>& hist_D2, std::string D1_filename, std::string D2_filename) {
  if (D1_filename.size() > 0) {
    std::ofstream fout(D1_filename);
    if (!fout.is_open()) {
      if (dist.verbose) {
        std::cerr << "\n[Error: Can't open file " << D1_filename << ".]" << std::endl;
      }
      return false;
    }
    else {
      for (auto& x:hist_D1) {
        fout << x.first << '\t' << x.second << '\n';
      }
      fout.close();
    }
  }

  if (D2_filename.size() > 0) {
    std::ofstream fout(D2_filename);
    if (!fout.is_open()) {
      if (dist.verbose) {
        std::cerr << "\n[Error: Can't open file " << D1_filename << ".]" << std::endl;
      }
      return false;
    }
    else {
      for (auto& x:hist_D2) {
        fout << x.first << '\t' << x.second << '\n';
      }
      fout.close();
    }
  }

  return true;
}

bool partition(dist_t& dist, int64_t d, std::string D1_filename, std::string D2_filename) {
  if (d > dist.N) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Invalid d value " << d << " is greater than number of samples " << dist.N << ". Nothing done.]" << std::endl;
    }
    return false;
  }

  if (d <= 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Invalid d value " << d << ". d must be a positive number. Nothing done.]" << std::endl;
    }
    return false;
  }

  if (d * 10 <= dist.N) {
    partition_small_d(dist, d);
  }
  else {
    partition_large_d(dist, d);
  }
  sort(dist.D2_idx.begin(), dist.D2_idx.end());

  if (dist.filetype == "freqcount") {
    if (D1_filename.size() != 0 || D2_filename.size() != 0) {
      if (dist.verbose) {
        std::cerr << "[Note: Samples in format 'freqcount', can't retrieve actual passwords. Partition done but nothing written to file(s).]" << std::endl;
      }
    }
    dist.D2_hist.clear();

    std::vector<int64_t> freqs;
    for (auto fc:dist.freqcount) {
      for (int j=0; j<fc.second; ++j) {
        freqs.push_back(fc.first);
      }
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(freqs.begin(), freqs.end(), gen);
    
    std::unordered_map<int64_t, int64_t> hist_D1;
    std::unordered_map<int64_t, int64_t> hist_D2;
    int64_t cumulative_freq = 0;
    int64_t i = 0;
    int64_t pwd_id = 1;

    for (int64_t pwd_id=0; pwd_id<freqs.size(); ++pwd_id) {
      cumulative_freq += freqs[pwd_id];
      int64_t in_D2_cnt = 0;
      while (i < dist.D2_idx.size() && dist.D2_idx[i] <= cumulative_freq) {
        ++i;
        ++in_D2_cnt;
      }
      if (freqs[pwd_id] - in_D2_cnt != 0) hist_D1[pwd_id] = (freqs[pwd_id] - in_D2_cnt);
      if (in_D2_cnt != 0) hist_D2[pwd_id] = in_D2_cnt;
    }

    std::vector<std::pair<int64_t, int64_t>> D1_pwdfreq;
    for (auto x:hist_D1) {
      D1_pwdfreq.push_back(x);
    }
    sort(D1_pwdfreq.begin(), D1_pwdfreq.end(), [&](std::pair<int64_t, int64_t>& a, std::pair<int64_t, int64_t>& b) {
      return a.second > b.second;
    });

    int64_t cur_hits = 0;
    dist.D1_attack_hits.clear();
    dist.distinct_D1 = D1_pwdfreq.size();
    for (int i=0; i<D1_pwdfreq.size() && cur_hits<d; ++i) {
      if (hist_D2[D1_pwdfreq[i].first] != 0) {
        cur_hits += hist_D2[D1_pwdfreq[i].first];
        dist.D1_attack_hits.push_back(std::make_pair(i+1, cur_hits));
      }
    }
  }
  else {
    std::unordered_map<std::string, int64_t> D1_hist;
    std::unordered_map<std::string, int64_t> D2_hist;
    count_in_partition(dist, D1_hist, D2_hist);
    dist.D2_hist = D2_hist;
    write_partition(dist, D1_hist, D2_hist, D1_filename, D2_filename);

    // precompute dictionary attack with D1
    std::vector<std::pair<std::string, int64_t>> D1_pwdfreq;
    for (auto x:D1_hist) {
      D1_pwdfreq.push_back(x);
    }
    sort(D1_pwdfreq.begin(), D1_pwdfreq.end(), [&](std::pair<std::string, int64_t>& a, std::pair<std::string, int64_t>& b) {
      return a.second > b.second;
    });

    int64_t cur_hits = 0;
    dist.D1_attack_hits.clear();
    dist.distinct_D1 = D1_pwdfreq.size();
    for (int i=0; i<D1_pwdfreq.size() && cur_hits<d; ++i) {
      if (D2_hist[D1_pwdfreq[i].first] != 0) {
        cur_hits += D2_hist[D1_pwdfreq[i].first];
        dist.D1_attack_hits.push_back(std::make_pair(i+1, cur_hits));
      }
    }
  }

  return true;
}

bool partition(dist_t& dist, double fraction, std::string D1_filename, std::string D2_filename) {
  if (fraction <= 0 || fraction > 1) {
    if (dist.verbose) {
      std::cerr << "\nError: Invalid fraction " << fraction << ". Nothing done." << std::endl;
    }
    return false;
  }
  return partition(dist, (int64_t) floor(fraction * dist.N), D1_filename, D2_filename);
}

bool pre_partition(dist_t& dist, int64_t d) {
  if (d > dist.N) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Invalid d value " << d << " is greater than number of samples " << dist.N << ". Nothing done.]" << std::endl;
    }
    return false;
  }

  if (d <= 0) {
    if (dist.verbose) {
      std::cerr << "\n[Error: Invalid d value " << d << ". d must be a positive number. Nothing done.]" << std::endl;
    }
    return false;
  }

  if (dist.filetype == "freqcount") {
    if (dist.verbose) {
      std::cerr << "\n[Error: Sample must be in format \"plain\" or \"pwdfreq\" to qualify for pre-partitioning. Nothing done.]" << std::endl;
    }
    return false;
  }

  dist.D2_idx.resize(d);
  for (int i=1; i<=d; ++i) {
    dist.D2_idx[i] = i;
  }

  std::unordered_map<std::string, int64_t> D1_hist;
  std::unordered_map<std::string, int64_t> D2_hist;
  count_in_partition(dist, D1_hist, D2_hist);
  dist.D2_hist = D2_hist;

  // precompute dictionary attack with D1
  std::vector<std::pair<std::string, int64_t>> D1_pwdfreq;
  for (auto x:D1_hist) {
    D1_pwdfreq.push_back(x);
  }
  sort(D1_pwdfreq.begin(), D1_pwdfreq.end(), [&](std::pair<std::string, int64_t>& a, std::pair<std::string, int64_t>& b) {
    return a.second > b.second;
  });

  int64_t cur_hits = 0;
  dist.D1_attack_hits.clear();
  dist.distinct_D1 = D1_pwdfreq.size();
  for (int i=0; i<D1_pwdfreq.size() && cur_hits<d; ++i) {
    if (D2_hist[D1_pwdfreq[i].first] != 0) {
      cur_hits += D2_hist[D1_pwdfreq[i].first];
      dist.D1_attack_hits.push_back(std::make_pair(i+1, cur_hits));
    }
  }

  return true;
}

void model_attack(dist_t& dist, std::string attack_filename) {
  if (dist.D2_idx.size() == 0) {
    std::cerr << "\nError: Must partitoin before attacking. Nothing done." << std::endl;
  }

  std::ifstream fattk(attack_filename);
  if (!fattk.is_open()) {
    std::cerr << "\nError: Can't open attack file " << attack_filename << ". Nothing done." << std::endl;
    return;
  }

  dist.model_attack_filename = attack_filename;
  dist.model_attack_hits.clear();

  std::string pwd;
  std::unordered_set<std::string> seen;
  int64_t guesses = 1;
  int64_t cur_hits = 0;
  while (getline(fattk, pwd)) {
    if (seen.insert(pwd).second && dist.D2_hist[pwd] != 0) {
      cur_hits += dist.D2_hist[pwd];
      dist.model_attack_hits.push_back(std::make_pair(guesses, cur_hits));
    }
    ++guesses;
  }
}

