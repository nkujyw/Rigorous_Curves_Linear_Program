import sys
import argparse
import os
from collections import Counter

def main():
    parser = argparse.ArgumentParser(description="用法: python script.py <输入文件> <输出文件>")
    parser.add_argument('input_file', help='输入文件的路径 (源数据)')
    parser.add_argument('output_file', help='输出文件的路径 (结果保存位置)')

    args = parser.parse_args()

    #  检查输入文件是否存在
    if not os.path.exists(args.input_file):
        print(f"错误: 找不到输入文件 '{args.input_file}'")
        sys.exit(1)


    # 3. 开始统计
    frequency_distribution = Counter()
    
    try:
        # errors='ignore' 防止因为密码中有奇怪的二进制字符导致脚本报错崩溃
        with open(args.input_file, 'r', encoding='utf-8', errors='ignore') as f_in:
            for line in f_in:
                line = line.rstrip('\n\r')
                if not line:
                    continue

                parts = line.rsplit(maxsplit=1)

                if len(parts) == 2:
                    _, freq_str = parts 
                    
                    if freq_str.isdigit():
                        frequency_distribution[int(freq_str)] += 1
                        
    except Exception as e:
        print(f"读取文件时发生未知错误: {e}")
        sys.exit(1)

    # 4. 写入结果
    print(f"正在写入: {args.output_file} ...")
    
    try:
        with open(args.output_file, 'w', encoding='utf-8') as f_out:
            # 按频数从大到小排序 (key=lambda x: x[0], reverse=True)
            for freq, count in sorted(frequency_distribution.items(), key=lambda x: x[0], reverse=True):
                f_out.write(f"{freq} {count}\n")
    except Exception as e:
        print(f"写入文件时发生错误: {e}")
        sys.exit(1)

    print("处理完成！")

if __name__ == "__main__":
    main()