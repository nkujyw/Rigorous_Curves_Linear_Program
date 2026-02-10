import os

def replace_string_in_files(folder_path, old_str, new_str, extensions=['.cpp', '.hpp', '.h']):
    """
    遍历文件夹，替换指定文件类型中的字符串，并强制转换为 UTF-8 编码以修复编译警告。
    """
    count = 0
    # 尝试的编码列表，解决 GBK/UTF-8 混用的问题
    encodings_to_try = ['utf-8', 'gbk', 'gb18030', 'cp936', 'latin-1']

    print(f"--- 开始处理: {folder_path} ---")
    print(f"--- 将 '{old_str}' 替换为 '{new_str}' ---")

    for root, dirs, files in os.walk(folder_path):
        for file in files:
            # 检查文件后缀
            if any(file.endswith(ext) for ext in extensions):
                file_path = os.path.join(root, file)
                content = None
                read_encoding = None

                # 1. 尝试读取文件（自动识别编码）
                for enc in encodings_to_try:
                    try:
                        with open(file_path, 'r', encoding=enc) as f:
                            content = f.read()
                            read_encoding = enc
                            break
                    except UnicodeDecodeError:
                        continue
                
                if content is None:
                    print(f"[跳过] 无法识别文件编码: {file}")
                    continue

                # 2. 执行替换
                if old_str in content:
                    new_content = content.replace(old_str, new_str)
                    
                    # 3. 写入文件 (统一保存为 UTF-8，解决 VS 的 C4819 警告)
                    try:
                        with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(new_content)
                        print(f"[已修改] {file} (原编码: {read_encoding} -> 保存为: utf-8)")
                        count += 1
                    except Exception as e:
                        print(f"[写入失败] {file}: {e}")
                else:
                    # 如果内容没变，但原编码不是 utf-8，建议也转存一下修复警告（可选）
                    if read_encoding != 'utf-8':
                         with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(content)
                         print(f"[格式修复] {file} (未发现目标字符串，但已转换为 utf-8)")

    print(f"--- 处理完成，共修改了 {count} 个文件 ---")

# ================= 配置区域 =================
# 目标文件夹名称 (如果是当前目录下的子文件夹)
target_folder = "./Liner_Programmer" 

# 要查找的旧字符串
target_old = "yahoo"

# 要替换的新字符串
target_new = "000webhost"
# ===========================================

if __name__ == "__main__":
    if os.path.exists(target_folder):
        replace_string_in_files(target_folder, target_old, target_new)
    else:
        print(f"错误：找不到文件夹 '{target_folder}'")