cmake_minimum_required(VERSION 3.16.3)
project(dida_triangulate_shootout)

include(FetchContent)

FetchContent_Declare(dida_geom
    GIT_REPOSITORY https://github.com/lievenvanderheide/dida_geom.git
    GIT_TAG origin/main)
FetchContent_MakeAvailable(dida_geom)

FetchContent_Declare(countries_geojson
    GIT_REPOSITORY https://github.com/lievenvanderheide/geo-countries.git
    GIT_TAG origin/master)
FetchContent_MakeAvailable(countries_geojson)

find_package(Catch2 3 REQUIRED)

add_subdirectory(libtess2)
add_subdirectory(seidel)
add_subdirectory(poly2tri)

add_executable(dida_triangulate_shootout
    countries_geojson.hpp
    countries_geojson.cpp
    triangulate_shootout.cpp
    validation.cpp
    validation.hpp)

target_link_libraries(dida_triangulate_shootout dida libtess2 seidel poly2tri Catch2::Catch2WithMain)

file(INSTALL ${countries_geojson_SOURCE_DIR}/data/countries.geojson DESTINATION data)