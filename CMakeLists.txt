cmake_minimum_required(VERSION 3.20)
project(jni)

include_directories(~/.sdkman/candidates/java/current/include ~/.sdkman/candidates/java/current/include/darwin)

set(CMAKE_CXX_STANDARD 14)

add_library(jni SHARED Image.h Image.cpp)
