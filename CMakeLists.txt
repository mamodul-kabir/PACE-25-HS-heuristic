cmake_minimum_required(VERSION 3.10)
project(Pace)

set(CMAKE_CXX_STANDARD 23)

# ====== NetworKit setup ======
include_directories(
    ${CMAKE_SOURCE_DIR}/networkit/include
    ${CMAKE_SOURCE_DIR}/networkit/extlibs/tlx
)

add_library(networkit SHARED IMPORTED)
set_target_properties(networkit PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/networkit/build/libnetworkit.so
)

# ====== HiGHS setup ======
set(HIGHS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/HiGHS/highs")
set(HIGHS_LIB_DIR "${CMAKE_SOURCE_DIR}/HiGHS/build/lib")

include_directories(${HIGHS_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/HiGHS/build)
link_directories(${HIGHS_LIB_DIR})

# ====== ONNX Runtime setup ======
set(ORT_DIR ${CMAKE_SOURCE_DIR}/onnxruntime-linux-x64-1.22.0)

include_directories(${ORT_DIR}/include)
link_directories(${ORT_DIR}/lib)

# ====== Source files ======
add_executable(pace
    src/main.cpp
    src/pace.cpp
    src/pace.h
    src/graph.cpp
    src/graph.h
    src/netkit.cpp
    src/netkit.h
    src/highs.cpp
    src/highs.h
    src/mlpredict.cpp
    src/mlpredict.h
    src/prune.h
    src/prune.cpp
    src/nusc.h
    src/nusc.cpp
    src/wscp.h
)

# ====== Link libraries ======
target_link_libraries(pace
    networkit
    highs
    onnxruntime
)