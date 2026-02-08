#include "plotting.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <cmath>

// bool tikzPlotFromFile(string fileName, )

bool tikz_plot(std::vector<std::vector<std::pair<int64_t, double>>>& data, std::vector<std::string>& style, std::vector<std::string>& legend, std::string filename) {
  if (data.size() != style.size() || data.size() != legend.size()) {
    std::cerr << "Error: 'data' and 'style' must be the same size. Nothing done." << std::endl;
    return false;
  }

  std::ofstream fout(filename);
  if (!fout.is_open()) {
    std::cerr << "Error: Can't open file " << filename << ". Nothing done." << std::endl;
    return false;
  }

  int64_t xmin=INT64_MAX, xmax=INT64_MIN;
  for (auto& bound:data) {
    for (auto& p:bound) {
      xmin = std::min(p.first, xmin);
      xmax = std::max(p.first, xmax);
    }
  }
  int64_t lo = (int64_t) ceil(log10((double) xmin));
  int64_t hi = (int64_t) floor(log10((double) xmax));

  fout << "\\begin{tikzpicture}\n"
          "  \\begin{axis}[\n"
          "    width=12cm,\n"
          "    height=10cm,\n"
          "    xmode=log,\n";
  fout << "    xmin=" << xmin << ", xmax=" << xmax << ",\n";

  fout << "    xtick={";
  int64_t p10 = 1;
  for (int i=0; i<lo; ++i) {
    p10 *= 10;
  }
  for (int64_t i=lo; i<=hi; ++i, p10*=10) {
    fout << p10 << ((i == hi) ? "" : ",");
  }
  fout << "},\n";

  fout << "    xticklabels={";
  for (int64_t i=lo; i<=hi; ++i) {
    fout << "$10^{" << i << ((i == hi) ? "}$" : "}$,");
  }
  fout << "},\n";

  fout << "    ymin=0.0, ymax=1.0,\n"
          "    xlabel={Number of Guesses ($G$)},\n"
          "    ylabel={Cracked Fraction ($\\lambda_G$)},\n"
          "    grid=major,\n"
          "    legend style={at={(1.02,1)}, anchor=north west},\n"
          "    legend cell align={left},\n"
          "    legend columns=1,\n"
          "  ]\n";

  for (int i=0; i<data.size(); ++i) {
    fout << "\\addplot[" << style[i] << "] coordinates {\n";
    for (auto &pt:data[i]) {
      fout << "(" << pt.first << "," << pt.second << ") ";
    }
    fout << "\n};\n"
            "\\addlegendentry{" << legend[i] << "}\n";
  }

  fout << "  \\end{axis}\n"
          "\\end{tikzpicture}\n";

  fout.close();

  return true;
}

