cmake_minimum_required(VERSION 3.10)

# C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PasswordGuessingCurves)

# 1. 查找 OpenMP
find_package(OpenMP REQUIRED)

# 全局设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 2. 扫描源文件
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# 3. Gurobi 路径配置 (Windows)
if(WIN32)
    set(GUROBI_PATH_ROOT "D:/Gurobi")
    set(GUROBI_LIB_NAME "gurobi120") 
    set(GUROBI_CPP_LIB_NAME "gurobi_c++md2017") 
else()
    # Linux/Mac 配置 (保留以防万一)
    set(GUROBI_PATH_ROOT "/opt/gurobi1200/linux64")
    set(GUROBI_LIB_NAME "gurobi120")
    set(GUROBI_CPP_LIB_NAME "gurobi_c++")
endif()

# 4. 自动查找库文件 (这是修复的关键)
# CMake 会自动找到这些 .lib 文件的绝对路径，而不是传递错误的 -l 参数
find_library(GUROBI_LIB 
    NAMES ${GUROBI_LIB_NAME}
    PATHS ${GUROBI_PATH_ROOT}/lib 
    NO_DEFAULT_PATH
)

find_library(GUROBI_CPP_LIB 
    NAMES ${GUROBI_CPP_LIB_NAME} gurobi_c++
    PATHS ${GUROBI_PATH_ROOT}/lib 
    NO_DEFAULT_PATH
)

if (NOT GUROBI_LIB OR NOT GUROBI_CPP_LIB)
    message(FATAL_ERROR "无法在 ${GUROBI_PATH_ROOT}/lib 找到 Gurobi 库文件！请检查路径是否正确，或者文件名是否匹配。")
endif()

message(STATUS "Found Gurobi Lib: ${GUROBI_LIB}")
message(STATUS "Found Gurobi C++ Lib: ${GUROBI_CPP_LIB}")

# 5. 生成可执行文件
add_executable(LPSolver Liner_Programmer/main.cpp ${SRC_FILES})

# 包含头文件
target_include_directories(LPSolver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(LPSolver PUBLIC ${GUROBI_PATH_ROOT}/include)

# 6. 链接库
# 直接传递找到的 .lib 文件路径
target_link_libraries(LPSolver ${GUROBI_CPP_LIB} ${GUROBI_LIB} OpenMP::OpenMP_CXX)