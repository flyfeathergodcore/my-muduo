#!/bin/bash
set -e

cd ..
rm -rf build/*
mkdir -p build
cd build
cmake ..
make
sudo make install  # 用 CMake 的安装功能替代手动复制
sudo ldconfig
echo "Installation completed successfully."