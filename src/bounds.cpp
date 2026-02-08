#include "bounds.hpp"

#include <iostream>
#include <cmath>

#include "helpers.hpp"
#include "error_check.hpp"

// LP paper

double freq_UB(dist_t& dist, int64_t G, double err) { // Coro 4
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }

  int64_t top_G_freq = most_frequent(dist, G);
  double eps = sqrt(-log(err) / (2.0 * dist.N));
  return std::min(((double) top_G_freq) / ((double) dist.N) + eps, 1.0);
}

double samp_LB(dist_t& dist, int64_t G, double err) { // Thm 5
  if (!error_check_with_partition(dist, G, err)) {
    return -1;
  }
  
  if (dist.D1_attack_hits.size() == 0 || dist.D1_attack_hits[0].first > G) {
    return 0.0;
  }

  int64_t lo = 0, hi = dist.D1_attack_hits.size() - 1, mid;
  while (lo != hi) {
    mid = (lo + hi + 1) >> 1;
    if (dist.D1_attack_hits[mid].first <= G) {
      lo = mid;
    }
    else {
      hi = mid - 1;
    }
  }
  int64_t h_D1_D2_G = dist.D1_attack_hits[lo].second;
  double t = sqrt(-log(err) * dist.D2_idx.size() / 2.0);

  return std::max(((double) h_D1_D2_G - t) / dist.D2_idx.size(), 0.0);
}

double extended_LB(dist_t& dist, int64_t G, double err) { // Coro 7
  if (!error_check_with_attack(dist, G, err)) {
    return -1;
  }

  int64_t G_remaining = G - dist.distinct_D1;

  if (dist.model_attack_hits.size() == 0 || dist.model_attack_hits[0].first > G_remaining) {
    return samp_LB(dist, G, err);
  }

  int64_t lo = 0, hi = dist.model_attack_hits.size() - 1, mid;
  while (lo != hi) {
    mid = (lo + hi + 1) >> 1;
    if (dist.model_attack_hits[mid].first <= G_remaining) {
      lo = mid;
    }
    else {
      hi = mid - 1;
    }
  }
  int64_t h_D1_D2_G = ((dist.D1_attack_hits.size() == 0) ? 0 : dist.D1_attack_hits.back().second) + dist.model_attack_hits[lo].second;
  double t = sqrt(-log(err) * dist.D2_idx.size() / 2.0);

  return ((double) h_D1_D2_G - t) / dist.D2_idx.size();
}

double prior_LB(dist_t& dist, int64_t G, int64_t j, double err1, double err2) { // Thm 9
  if (!error_check_prior_LB(dist, G, j, err1, err2)) {
    return -1;
  }

  double L = ((double) G) / ((double) dist.N);
  double temp = (double) dist.N;
  for (int i=0; i<j-1; ++i) {
    temp /= ((j-1) * L);
  }

  int lo = 0, hi = dist.freqcount.size() - 1, mid;
  while (lo != hi) {
    mid = (lo + hi + 1) >> 1;
    if (dist.freqcount[mid].first >= j) {
      lo = mid;
    }
    else {
      hi = mid - 1;
    }
  }

  double f_SL = ((double) dist.preftotal[lo]) / ((double) dist.N) - temp;
  double t = sqrt(-dist.N * j * j * log(err1) / 2.0);
  double eps = sqrt(-log(err2) / (2.0 * dist.N));

  return std::max(f_SL - t/dist.N - eps, 0.0);
}

double prior_LB(dist_t& dist, int64_t G, int64_t j, double err) { // Thm 9
  if (!error_check_prior_LB(dist, G, j, err, err)) {
    return -1;
  }
  return prior_LB(dist, G, j, err/2, err/2);
}

double best_prior_LB(dist_t& dist, int64_t G, double err1, double err2) { // Thm 9
  if (!error_check_prior_LB(dist, G, 2, err1, err2)) {
    return -1;
  }

  double res = 0.0;
  for (int64_t j=2; j<=1000; ++j) {
    res = std::max(res, prior_LB(dist, G, j, err1, err2));
  }
  return res;
}

double best_prior_LB(dist_t& dist, int64_t G, double err) {
  if (!error_check_prior_LB(dist, G, 2, err, err)) {
    return -1;
  }
  return best_prior_LB(dist, G, err/2, err/2);
}

// PIN paper

double binom_LB(dist_t& dist, int64_t G, double err) { // Coro 4
  if (!error_check_with_partition(dist, G, err)) {
    return -1;
  }

  if (dist.D1_attack_hits.size() == 0 || dist.D1_attack_hits[0].first > G) {
    return 0.0;
  }

  int64_t lo = 0, hi = dist.D1_attack_hits.size() - 1, mid;
  while (lo != hi) {
    mid = (lo + hi + 1) >> 1;
    if (dist.D1_attack_hits[mid].first <= G) {
      lo = mid;
    }
    else {
      hi = mid - 1;
    }
  }
  int64_t cracked = dist.D1_attack_hits[lo].second;
  int64_t d = dist.D2_idx.size();

  int64_t iterations = 50;
  double lo2 = 0.0, hi2 = 1.0, mid2;

  while (iterations--) {
    mid2 = (lo2 + hi2) / 2.0;
    double cdf = 1.0 - bcdf(cracked - 1, d, mid2);
    if (cdf > err) {
      hi2 = mid2;
    }
    else {
      lo2 = mid2;
    }
  }

  return mid2;
}

double binom_UB(dist_t& dist, int64_t G, double err) { // Thm 2
  if (!error_check_basic(dist, G, err)) {
    return -1;
  }

  int64_t F = most_frequent(dist, G);
  if (F == dist.N) {
    return 1.0;
  }

  int64_t iterations = 50;
  double lo = 0.0, hi = 1.0, mid;

  while (iterations--) {
    mid = (lo + hi) / 2.0;
    double cdf = bcdf(F, dist.N, mid);
    if (cdf > err) {
      lo = mid;
    }
    else {
      hi = mid;
    }
  }

  return mid;
}

