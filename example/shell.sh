#!/bin/bash

set -e  # 遇到错误立即退出

cd ..

# 清理并创建build目录（修正语法错误）
rm -rf "$(pwd)/build"/*  # 使用$(...)代替`...`，更易读
if [ ! -d "$(pwd)/build" ]; then  # 注意[ ]前后的空格，以及!后的空格，-d参数
    mkdir -p "$(pwd)/build"  # -p确保父目录存在，避免错误
fi

# 编译项目
cd "$(pwd)/build" &&  # 简化路径写法
    cmake .. &&
    make

cd ..  # 返回项目根目录

# 安装头文件
if [ ! -d "/usr/include/mymudou" ]; then  # 注意-d参数和空格
    sudo mkdir -p "/usr/include/mymudou"  # /usr目录需要root权限
fi

# 复制所有.h头文件（避免ls的潜在问题，改用通配符）
for header in *.h; do
    # 跳过非文件（如目录，防止误操作）
    if [ -f "$header" ]; then
        sudo cp "$header" "/usr/include/mymudou/"
    fi
done

# 安装动态库
sudo cp "$(pwd)/lib/libmymuduo.so" "/usr/lib/"  # 确保路径正确（假设编译后生成在build/lib）

# 更新动态链接库缓存
sudo ldconfig

cd "$(pwd)/example"
if [ ! -d "$(pwd)/testserver" ]; then
    rm -rf "$(pwd)/testserver"
fi

make
