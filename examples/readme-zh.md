这是一个关于如何使用该工具分析密码样本的示例。首先，包含必要的文件。

    #include <iostream>

    #include "distribution.hpp"
    #include "pwdio.hpp"
    #include "bounds.hpp"
    #include "lp_bounds.hpp"
    #include "wrappers.hpp"
    #include "plotting.hpp"

在主函数中，首先将样本文件读取到 `dist_t` 对象中。使用 if 语句检查样本是否读取成功。

    dist_t yahoo_dist;
    if (read_file(yahoo_dist, "./dataset/yahoo_freqcount.txt", "freqcount")) {
      // 成功读取样本
    }
    else {
      // 处理读取文件失败的情况
    }

然后，将样本划分为两个集合，这里我们选择 $d = 30000$ 作为 $D_{2}$ 的大小。

    partition(yahoo_dist, (int64_t) 30000);

以下几行代码展示了如何计算特定 $G$ 值的界限。

    int64_t G1 = 500000;
    int64_t G2 = 20000000000;
    double err = 0.01;
    std::cout << freq_UB(yahoo_dist, G1, err) << std::endl;
    std::cout << binom_LB(yahoo_dist, G2, err) << std::endl;

结果为 `0.401` 和 `0.569`，表明 $\Pr[\lambda_{5 \cdot 10^{5}} \leq 0.401] \geq 0.99$ 且 $\Pr[\lambda_{2 \cdot 10^{10}} \geq 0.569] \geq 0.99$。

以下几行展示了如何设置线性规划界限的参数。

    int64_t total_samples = yahoo_dist.N;
    double q = 1.004;
    int64_t iprime = 5;
    std::vector<double> errs = {0.001, 0.0015, 0.0018, 0.0019, 0.002, 0.002};
    std::vector<double> xhats = {3.0/total_samples, 4.0/total_samples, 5.0/total_samples, 6.0/total_samples, 7.0/total_samples, 8.0/total_samples};
    int64_t G3 = 2000000000LL;
    std::cout << LP_LB(yahoo_dist, G3, q, iprime, errs, xhats) << std::endl;

结果是 `0.577`，表明 $\Pr[\lambda_{2 \cdot 10^{9}} \geq 0.577] \geq 0.99$。

以下几行演示了如何利用 `tikz_plot()` 函数生成整个猜测曲线的绘图。

    std::vector<std::string> style = {"red,dashed,thick", "blue,dashed,thick", "cyan,dashed,thick", "magenta,dashed,thick", "gray,dashed,thick"};
    std::vector<std::string> legend = {"binom LB", "binom UB", "LP LB", "LP UB", "Distinct(S)"};
    std::vector<std::vector<std::pair<int64_t, double>>> data(5);
    for (int64_t G = 50; G < yahoo_dist.distinct; G *= 1.6) {
      std::cout << G << std::endl;
      data[0].push_back(std::make_pair(G, binom_LB(yahoo_dist, G, 0.01)));
      data[1].push_back(std::make_pair(G, binom_UB(yahoo_dist, G, 0.01)));
      data[2].push_back(std::make_pair(G, LP_LB(yahoo_dist, G, 0.01)));
      data[3].push_back(std::make_pair(G, LP_UB(yahoo_dist, G, 0.01)));
    }
    for (int64_t G = yahoo_dist.distinct; G <= 10000000000LL; G *= 1.6) {
      std::cout << G << std::endl;
      data[0].push_back(std::make_pair(G, binom_LB(yahoo_dist, G, 0.01)));
      data[1].push_back(std::make_pair(G, binom_UB(yahoo_dist, G, 0.01)));
      data[2].push_back(std::make_pair(G, LP_LB(yahoo_dist, G, 0.01)));
      data[3].push_back(std::make_pair(G, LP_UB(yahoo_dist, G, 0.01)));
    }
    data[4].push_back(std::make_pair(yahoo_dist.distinct, 0.0));
    data[4].push_back(std::make_pair(yahoo_dist.distinct, 1.0));

    tikz_plot(data, style, legend, "output/plot.txt");

生成的绘图如下：

![plot](./plot.png)

最后，如果线性规划界限的返回值为 `-2`，则意味着优化问题不可行（无解），表明样本可能不是从分布中 *iid*（独立同分布）采样的。在这种情况下，linkedin 数据集包含了同一用户的重复账户。

    dist_t linkedin_dist;
    read_file(linkedin_dist, "./dataset/linkedin_freqcount.txt", "freqcount");
    double result = LP_LB(linkedin_dist, (int64_t) 500000, 0.01);
    if (result == -2) {
      // infeasible LP (线性规划不可行)
      std::cout << "Sample might not be iid!" << std::endl;
    }