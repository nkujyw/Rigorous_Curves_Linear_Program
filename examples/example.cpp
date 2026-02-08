#include <iostream>

#include "distribution.hpp"
#include "pwdio.hpp"
#include "bounds.hpp"
#include "lp_bounds.hpp"
#include "wrappers.hpp"
#include "plotting.hpp"
#include <fstream>

int main() {
  // plot("presentation/probs_P1000_B1.txt");
  // create the dist_t object and reading the sample file
  dist_t yahoo_dist;
  if (read_file(yahoo_dist, "./dataset/yahoo_freqcount.txt", "freqcount")) {
    // successfully read sample

    std::ofstream myFile("optimal-bounds.txt",std::ofstream::app);

    // partitioning -- will not be used in the lp bounds
    partition(yahoo_dist, (int64_t) 30000);

    // calculating bounds for individual guessing budgets
    // int64_t G1 = 500000;
    // int64_t G2 = 20000000000;
    // double err = 0.01;
    // std::cout << freq_UB(yahoo_dist, G1, err) << std::endl;
    // std::cout << binom_LB(yahoo_dist, G2, err) << std::endl;

    // setting parameters for the linear program
    int64_t total_samples = yahoo_dist.N;
    // double q = 1.004;
    // int64_t iprime = 5;
    std::vector<double> errs = {0.001, 0.0015, 0.0018, 0.0019, 0.002, 0.002};
    std::vector<double> xhats = {3.0/total_samples, 4.0/total_samples, 5.0/total_samples, 6.0/total_samples, 7.0/total_samples, 8.0/total_samples};
    // int64_t G3 = 2000000000LL;
    // std::cout << LP_LB(yahoo_dist, G3, q, iprime, errs, xhats) << std::endl;

    // std::vector<std::pair<int, double>> minData;
    // std::vector<std::pair<int, double>> maxData;
    for  (int64_t G = 2000; G < 4000; G += 1) {

      std::cout << "Guessing Number " << G << std::endl;
      // double currMin = std::max(LP_LB(yahoo_dist, G, 0.01),binom_LB(yahoo_dist, G, 0.01));
      // double currMax = std::min(LP_UB(yahoo_dist, G, 0.01),binom_UB(yahoo_dist, G, 0.01));
      double currMin = binom_LB(yahoo_dist, G, 0.01);
      double currMax = binom_UB(yahoo_dist, G, 0.01);
      std::cout << "min: " << std::to_string(currMin) << std::endl;
      std::cout << "max: " << std::to_string(currMax) << std::endl;
      // minData.push_back(std::make_pair(G, LP_LB(yahoo_dist, G, 0.01)));
      // maxData.push_back(std::make_pair(G, LP_UB(yahoo_dist, G, 0.01)));
      myFile << G << "," << std::to_string(currMin) << "," << std::to_string(currMax) << std::endl;
    // }

    // for (int64_t G = yahoo_dist.distinct; G <= 10000000000LL; G *= 1.6) {
    //   std::cout << G << std::endl;
    //   minData.push_back(std::make_pair(G, LP_LB(yahoo_dist, G, 0.01)));
    //   maxData.push_back(std::make_pair(G, LP_UB(yahoo_dist, G, 0.01)));
    //   myFile << G << minData.back().second << " " << maxData.back().second << std::endl;
    }

    myFile.close();

    // plotting the entire guessing curve
    // std::vector<std::string> style = {"red,dashed,thick", "blue,dashed,thick", "cyan,dashed,thick", "magenta,dashed,thick", "gray,dashed,thick"};
    // std::vector<std::string> legend = {"binom LB", "binom UB", "LP LB", "LP UB", "Distinct(S)"};
    // std::vector<std::vector<std::pair<int64_t, double>>> data(5);
    // for (int64_t G = 50; G < yahoo_dist.distinct; G *= 1.6) {
    //   std::cout << G << std::endl;
    //   data[0].push_back(std::make_pair(G, binom_LB(yahoo_dist, G, 0.01)));
    //   data[1].push_back(std::make_pair(G, binom_UB(yahoo_dist, G, 0.01)));
    //   data[2].push_back(std::make_pair(G, LP_LB(yahoo_dist, G, 0.01)));
    //   data[3].push_back(std::make_pair(G, LP_UB(yahoo_dist, G, 0.01)));
    // }
  //   for (int64_t G = yahoo_dist.distinct; G <= 10000000000LL; G *= 1.6) {
  //     std::cout << G << std::endl;
  //     data[0].push_back(std::make_pair(G, binom_LB(yahoo_dist, G, 0.01)));
  //     data[1].push_back(std::make_pair(G, binom_UB(yahoo_dist, G, 0.01)));
  //     data[2].push_back(std::make_pair(G, LP_LB(yahoo_dist, G, 0.01)));
  //     data[3].push_back(std::make_pair(G, LP_UB(yahoo_dist, G, 0.01)));
  //   }
  //   data[4].push_back(std::make_pair(yahoo_dist.distinct, 0.0));
  //   data[4].push_back(std::make_pair(yahoo_dist.distinct, 1.0));

    // tikz_plot(data, style, legend, "output/plot.txt");
  } else {
    // failed reading sample
    std::cout << "Couldn't read from file." << std::endl;
  }

  // // demonstration of infeasible LP
  // dist_t linkedin_dist;
  // read_file(linkedin_dist, "./dataset/linkedin_freqcount.txt", "freqcount");

  // double result = LP_LB(linkedin_dist, (int64_t) 500000, 0.01);
  // if (result == -2) {
  //   // infeasible LP
  //   std::cout << "Sample might not be iid!" << std::endl;
}

