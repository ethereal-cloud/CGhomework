#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
将 GB2312/GBK 编码的源文件转换为 UTF-8
"""

import os
import glob

# 项目根目录
root_dir = os.path.dirname(os.path.abspath(__file__))

# 需要处理的目录（排除 third_party）
source_dirs = ['common', 'fluid2d', 'fluid3d', 'ui']

# 文件扩展名
extensions = ['*.cpp', '*.h', '*.hpp']

def convert_file(filepath):
    """将单个文件从 GB2312/GBK 转换为 UTF-8"""
    try:
        # 尝试用 GBK 读取
        with open(filepath, 'r', encoding='gbk') as f:
            content = f.read()

        # 用 UTF-8 写回
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)

        print(f"[OK] {filepath}")
        return True
    except UnicodeDecodeError:
        # 如果 GBK 解码失败，可能已经是 UTF-8
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            print(f"[SKIP] {filepath} (already UTF-8)")
            return False
        except:
            print(f"[ERROR] {filepath} (unknown encoding)")
            return False
    except Exception as e:
        print(f"[ERROR] {filepath}: {e}")
        return False

def main():
    converted_count = 0
    skipped_count = 0
    error_count = 0

    for source_dir in source_dirs:
        dir_path = os.path.join(root_dir, source_dir)
        if not os.path.exists(dir_path):
            continue

        for ext in extensions:
            pattern = os.path.join(dir_path, '**', ext)
            for filepath in glob.glob(pattern, recursive=True):
                result = convert_file(filepath)
                if result:
                    converted_count += 1
                else:
                    skipped_count += 1

    # 处理根目录的 code.cpp 和 code.h
    for filename in ['code.cpp', 'code.h']:
        filepath = os.path.join(root_dir, filename)
        if os.path.exists(filepath):
            result = convert_file(filepath)
            if result:
                converted_count += 1
            else:
                skipped_count += 1

    print(f"\n转换完成！")
    print(f"  已转换: {converted_count} 个文件")
    print(f"  已跳过: {skipped_count} 个文件")

if __name__ == '__main__':
    main()
