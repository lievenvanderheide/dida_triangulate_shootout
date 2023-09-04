cmake_minimum_required(VERSION 3.16.3)
project(dida_triangulate_shootout)

include(FetchContent)

FetchContent_Declare(dida_geom
    GIT_REPOSITORY https://github.com/lievenvanderheide/dida_geom.git
    GIT_TAG origin/main)
FetchContent_MakeAvailable(dida_geom)

add_subdirectory(libtess2)

find_package(CGAL REQUIRED)

add_executable(dida_triangulate_shootout
    countries_geojson.hpp
    countries_geojson.cpp
    triangulate_shootout.cpp
    validation.cpp
    validation.hpp)

target_link_libraries(dida_triangulate_shootout dida libtess2 CGAL::CGAL)