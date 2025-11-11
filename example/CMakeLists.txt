# example/CMakeLists.txt
add_executable(testserver testserver.cpp)  # 生成名为 example_main 的可执行文件
target_link_libraries(testserver mymuduo)  # 链接核心库 mymuduo
target_include_directories(testserver PRIVATE ${PROJECT_SOURCE_DIR}/include)  # 配置头文件搜索路径