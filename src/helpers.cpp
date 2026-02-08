#include "helpers.hpp"

#include <vector>
#include <cmath>
#include <iostream>

std::vector<double> logs = {0};
std::vector<double> logpref = {0};

double fpow(double a, int64_t p) {
  double res = 1.0;
  for (; p; a=a*a, p<<=1) {
    if (p&1) {
      res *= a;
    }
  }
  return res;
}

void populate_logs(int64_t N) {
  if (logs.size() > N) {
    return;
  }
  int64_t old_size = logs.size();
  logs.resize(N+1);
  logpref.resize(N+1);
  for (int64_t i=old_size; i<=N; ++i) {
    logs[i] = log2((double) i);
    logpref[i] = logpref[i-1] + logs[i];
  }
}

double logbpdf(int64_t i, int64_t N, double p) {
  populate_logs(N);
  return logpref[N] - logpref[i] - logpref[N-i] + log2(p)*i + log2(1-p)*(N-i);
}

double bpdf(int64_t i, int64_t N, double p) {
  return pow(2, logbpdf(i, N, p));
}

double bcdf_direct(int64_t i, int64_t N, double p) {
  double res = 0.0;
  for (int j=0; j<i; ++j) {
    res += bpdf(j, N, p);
  }
  return res;
}

double bcdf_normal_estimate(int64_t i, int64_t N, double p) {
  double mean = N * p;
  double var = N * p * (1-p);
  return 0.5 * std::erfc(-((i + 0.5 - mean) / sqrt(var)) / sqrt((double) 2.0));
}

double bcdf(int64_t i, int64_t N, double p) {
  return bcdf_normal_estimate(i, N, p);
}

// struct simpson {
//   double area(double l, double r, double fl, double fm, double fr) {
//     return (fl + 4 * fm + fr) * (r - l) / 6;
//   }
//   double solve(double (*f)(double), double l, double m, double r, double eps, double fl, double fm, double fr, double a) {
//     double lm = l + (m - l) / 2, rm = m + (r - m) / 2;
//     double flm = f(lm), frm = f(rm);
//     double left = area(l, m, fl, flm, fm), right = area(m, r, fm, frm, fr);
//     if (fabs(left + right - a) <= 15 * eps) {
//       return left + right + (left + right - a) / 15.0;
//     }
//     return solve(f, l, lm, m, eps / 2, fl, flm, fm, left) + solve(f, m, rm, r, eps / 2, fm, frm, fr, right);
//   }
//   double solve(double (*f)(double), double l, double r, double eps) {
//     double m = l + (r - l) / 2;
//     double fl = f(l), fm = f(m), fr = f(r);
//     return solve(f, l, m, r, eps, fl, fm, fr, area(l, r, fl, fm, fr));
//   }
// }

