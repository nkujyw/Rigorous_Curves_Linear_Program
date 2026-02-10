#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "distribution.hpp"
#include "pwdio.hpp"
#include "lp_bounds.hpp"

// 辅助函数：提取文件名
std::string get_filename_stem(const std::string& path) {
    size_t last_slash = path.find_last_of("/\\");
    size_t last_dot = path.find_last_of(".");
    size_t start = (last_slash == std::string::npos) ? 0 : last_slash + 1;
    size_t count = (last_dot == std::string::npos) ? std::string::npos : last_dot - start;
    return path.substr(start, count);
}

int main() {
    std::cout << "=== Program Started (Single Threaded) ===" << std::endl;

    
    std::string file_path = "./dataset/000webhost_freqcount.txt";
    std::string dataset_name = "000webhost"; 

    // 1. 读取数据
    dist_t dist;
    std::cout << "[Info] Reading file: " << file_path << " ..." << std::endl;
    if (!read_file(dist, file_path, "freqcount")) {
        std::cerr << "[Error] Failed to read file! Check path." << std::endl;
        return 1;
    }
    std::cout << "[Info] File read successfully. N = " << dist.N << std::endl;

    // 2. 定义参数
    std::vector<int64_t> G_list = {65536, 131072, 262144, 524288};
    double err = 0.01;
    
    // 准备容器存储结果
    std::vector<double> results_lb(G_list.size());
    std::vector<double> results_ub(G_list.size());

    std::cout << "Starting calculation for " << G_list.size() << " points..." << std::endl;

    // 3. 循环计算
    for (int i = 0; i < (int)G_list.size(); ++i) {
        int64_t current_G = G_list[i];

        std::cout << "Processing G = " << current_G << " ..." << std::endl;

        try {
            // 计算 LB
            double lb = LP_LB_slow(dist, current_G, err);
            // 计算 UB
            double ub = LP_UB_slow(dist, current_G, err);

            // 存储结果
            results_lb[i] = lb;
            results_ub[i] = ub;

            std::cout << "[DONE] G = " << current_G 
                      << " -> LB: " << lb << ", UB: " << ub << std::endl;

        } catch (std::exception& e) {
            std::cerr << "\n[ERROR] Failed at G=" << current_G << ": " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "\n[ERROR] Crashed at G=" << current_G << "!" << std::endl;
        }
    }

    std::cout << "All calculations finished. Appending to files..." << std::endl;

    // 4. 写入文件 (追加模式)
    std::string lb_filename = dataset_name + "_LB.txt";
    std::string ub_filename = dataset_name + "_UB.txt";

   
    std::ofstream lb_file(lb_filename, std::ios::app);
    std::ofstream ub_file(ub_filename, std::ios::app);

    if (lb_file.is_open() && ub_file.is_open()) {
        for (size_t i = 0; i < G_list.size(); ++i) {
            lb_file << G_list[i] << " " << results_lb[i] << "\n";
            ub_file << G_list[i] << " " << results_ub[i] << "\n";
        }
        std::cout << "Results appended to " << lb_filename << " and " << ub_filename << std::endl;
    } else {
        std::cerr << "[Error] Could not open output files for writing." << std::endl;
    }

    return 0;
}