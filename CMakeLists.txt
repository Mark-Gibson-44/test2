cmake_minimum_required(VERSION 3.10)

project(fyp_opt)

find_package(LLVM REQUIRED CONFIG)

add_executable(fyp_opt main.cpp opt.cpp)




include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader)




target_include_directories(fyp_opt PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(fyp_opt ${llvm_libs})
#target_include_directories(Run_mutate PUBLIC "${PROJECT_BINARY_DIR}")
#target_link_libraries(Run_mutate ${llvm_libs})
