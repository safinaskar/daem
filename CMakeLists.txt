cmake_minimum_required (VERSION 3.0.2) # 3.0.2 подходит
project (daem LANGUAGES CXX)
add_executable (daem main.cpp)
target_link_libraries (daem -lsh)
install (TARGETS daem DESTINATION bin)
