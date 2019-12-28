cmake_minimum_required(VERSION 2.6)
project (src/SSSP)
add_definitions(-std=c++11)

project(src/SSSP CXX)

ENABLE_LANGUAGE(C)

# find mpi
find_package(MPI REQUIRED)

set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS}")
include_directories(${MPI_INCLUDE_PATH})
link_libraries(${MPI_LIBRARIES})

# find OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake/FindTBB/")

find_package(TBB)

if(TBB_FOUND)
    message(STATUS "TBB found: " ${TBB_INCLUDE_DIRS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltbb -ltbbmalloc")
    include_directories(${TBB_INCLUDE_DIRS})
    link_libraries(${TBB_LIBRARIES})
endif(TBB_FOUND)


find_package(Boost COMPONENTS program_options system thread)

if(Boost_FOUND)
    message(STATUS "Boost found: " ${Boost_INCLUDE_DIR})
    include_directories( ${Boost_INCLUDE_DIR})
    link_libraries(${Boost_LIBRARIES})
endif(Boost_FOUND)

set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -g -Wall -fsigned-char -D TBB_USE_THREADING_TOOLS")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O3 -Wall -fsigned-char")

set(CMAKE_BUILD_TYPE Release)

#add_executable(src/SSSP src/SSSP.cpp)
#add_executable(src/deltaab src/delta_ab_dis.cpp)
#add_executable(src/absssp src/ab_sssp.cpp)
#add_executable(src/bellman src/bellman_ford.cpp)
#add_executable(src/beldel src/beldel.cpp)
#add_executable(simple_sssp simple_sssp.cpp)
#add_executable(src/deltastep src/delta_step.cpp)
add_executable(src/test src/test.cpp)
