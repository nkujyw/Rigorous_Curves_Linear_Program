import os

def replace_string_in_files(folder_path, old_str, new_str, extensions=['.cpp', '.hpp', '.h']):
    """
    遍历文件夹，替换指定文件类型中的字符串
    """
    count = 0
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

                for enc in encodings_to_try:
                    try:
                        with open(file_path, 'r', encoding=enc) as f:
                            content = f.read()
                            read_encoding = enc
                            break
                    except UnicodeDecodeError:
                        continue
                
                if content is None:
                    print(f"无法识别文件编码: {file}")
                    continue


                if old_str in content:
                    new_content = content.replace(old_str, new_str)
                    
                    
                    try:
                        with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(new_content)
                        print(f"[已修改] {file} (原编码: {read_encoding} -> 保存为: utf-8)")
                        count += 1
                    except Exception as e:
                        print(f"[写入失败] {file}: {e}")
                else:
                    if read_encoding != 'utf-8':
                         with open(file_path, 'w', encoding='utf-8') as f:
                            f.write(content)
                         print(f"{file} (未发现目标字符串，但已转换为 utf-8)")

    print(f"--- 处理完成，共修改了 {count} 个文件 ---")


target_folder = "./Linear_Programming" 

# 要查找的旧字符串
target_old = "yahoo"

# 要替换的新字符串
target_new = "rockyou"

if __name__ == "__main__":
    if os.path.exists(target_folder):
        replace_string_in_files(target_folder, target_old, target_new)
    else:
        print(f"错误：找不到文件夹 '{target_folder}'")