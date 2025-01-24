cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(RemoteUnlock)

idf_build_set_property(CXX_COMPILE_OPTIONS "-Wno-missing-field-initializers" APPEND)
