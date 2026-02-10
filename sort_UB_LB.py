import os
import glob

def sort_file_inplace(file_path):
    """
    读取文件，按第一列数值排序，然后覆盖原文件。
    """
    try:
        # 1. 读取文件内容
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()

        # 2. 解析数据
        # 格式: list of (num1, full_line_string)
        data = []
        for line in lines:
            stripped_line = line.strip()
            if not stripped_line:
                continue # 跳过空行
            
            parts = stripped_line.split()
            try:
                num1 = int(parts[0]) 
                data.append((num1, line))
            except ValueError:
                print(f"  [警告] 跳过格式错误行: {stripped_line[:20]}...")
                continue


        data.sort(key=lambda x: x[0])

        with open(file_path, 'w', encoding='utf-8') as f:
            for _, original_line in data:
                f.write(original_line)
        
        print(f"✅ 已排序: {os.path.basename(file_path)}")

    except Exception as e:
        print(f"❌ 处理文件 {os.path.basename(file_path)} 时出错: {e}")

def batch_process(folder_path):

    search_pattern = os.path.join(folder_path, "*.txt")
    files = glob.glob(search_pattern)
    
    total_files = len(files)
    print(f"--- 开始处理 ---")
    print(f"目标文件夹: {folder_path}")
    print(f"发现文件数: {total_files}")
    print(f"----------------")

    if total_files == 0:
        print("未找到 .txt 文件，请检查路径。")
        return

    for i, file_path in enumerate(files, 1):
        sort_file_inplace(file_path)

    print(f"----------------")
    print(f"所有任务完成！")

if __name__ == "__main__":
    target_folder = './LB_UB_dataset' 
    
    if os.path.exists(target_folder):
        batch_process(target_folder)
    else:
        print(f"错误: 文件夹 '{target_folder}' 不存在。")