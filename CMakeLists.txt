cmake_minimum_required(VERSION 3.10)
project(mymuduo LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置库文件输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(src SRC_LIST)

add_library(mymuduo SHARED ${SRC_LIST})

target_include_directories(mymuduo
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION include/mymuduo  # 增加 mymuduo 子目录
)

install(
    TARGETS mymuduo
    LIBRARY DESTINATION lib
)

# 如果你的库依赖其他系统库（如 pthread），也在这里链接
# target_link_libraries(mymuduo pthread)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
# add_subdirectory(example)  
