cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(PasswordGuessingCurves)

find_package(OpenMP REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

file(GLOB_RECURSE SRC_FILES src/*.cpp)

# 3. Gurobi 路径配置 (Windows)
if(WIN32)
    set(GUROBI_PATH_ROOT "D:/Gurobi")
    set(GUROBI_LIB_NAME "gurobi120") 
    set(GUROBI_CPP_LIB_NAME "gurobi_c++md2017") 
else()
    # Linux/Mac 配置
    set(GUROBI_PATH_ROOT "/opt/gurobi1200/linux64")
    set(GUROBI_LIB_NAME "gurobi120")
    set(GUROBI_CPP_LIB_NAME "gurobi_c++")
endif()

# 4. 自动查找库文件
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
    message(FATAL_ERROR "无法在 ${GUROBI_PATH_ROOT}/lib 找到 Gurobi 库文件！请检查路径是否正确。")
endif()

message(STATUS "Found Gurobi Lib: ${GUROBI_LIB}")
message(STATUS "Found Gurobi C++ Lib: ${GUROBI_CPP_LIB}")

# 5. 循环生成 10 个可执行文件
# 文件名为 main1.cpp, main2.cpp, ..., main10.cpp
# 生成的可执行文件名为 LPSolver1, LPSolver2, ..., LPSolver10

foreach(i RANGE 0 10)
    set(TARGET_NAME "LPSolver${i}")
    set(SOURCE_FILE "Liner_Programmer/main${i}.cpp")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
        message(STATUS "Configuring executable: ${TARGET_NAME} with source ${SOURCE_FILE}")

        add_executable(${TARGET_NAME} ${SOURCE_FILE} ${SRC_FILES})
        target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_include_directories(${TARGET_NAME} PUBLIC ${GUROBI_PATH_ROOT}/include)
        target_link_libraries(${TARGET_NAME} ${GUROBI_CPP_LIB} ${GUROBI_LIB} OpenMP::OpenMP_CXX)
    else()
        message(WARNING "Source file ${SOURCE_FILE} not found! Skipping ${TARGET_NAME}.")
    endif()
endforeach()