#pragma once

#include <stdint.h>

double fpow(double, int64_t);
void populate_logs(int64_t);
double logbpdf(int64_t, int64_t, double);
double bpdf(int64_t, int64_t, double);
double bcdf_direct(int64_t, int64_t, double);
double bcdf_normal_estimate(int64_t, int64_t, double);
double bcdf(int64_t, int64_t, double);
