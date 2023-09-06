# Triangulation shootout

This repository contains the benchmark which compares DidaGeom's `triangulate` implementation against various other implementations. As can be seen in the table below, our implementation is the clear winner.

The other implementations are

* [libtess2](https://github.com/memononen/libtess2).
* [earcut.hpp](https://github.com/mapbox/earcut.hpp).
* [Seidel](http://gamma.cs.unc.edu/SEIDEL/).

## Results
The polygons we're triangulating are countries taken from the [geo-countires](https://github.com/datasets/geo-countries) dataset. The numbers in parentheses in the table below are the number of vertices of each polygon.

library      | Canada (20058) | Chile (7288)    | Bangladesh (1828) | Netherlands (592) | San Marino (18) |
------------ | -------------- | --------------- | ----------------- | ----------------- | --------------- |
DidaGeom     | 837 μs         | 292 μs          | 68 μs             | 16.8 μs           | 661 ns          |
libtess2     | 236946 μs      | 27374 μs        | 2909 μs           | 438 μs            | 11229 ns        |
earcut.hpp   | 9402 μs        | 2488 μs         | 560 μs            | 139 μs            | 815.4 ns        |
Seidel       | 67762 μs       | 18551 μs        | 12497 μs          | 11513 μs          | 10705 ns        |
poly2tri     | 79118 μs       | 13643 μs        | 3290 μs           | 1050 μs           | 16934 ns        |
std::sort    | 2241 μs        | 737 μs          | 189 μs            | 72 μs             | 448 ns          |

So DidaGeom's implementation is the clear winner, with `earcut.hpp` coming in second at approximately 8 to 10 times slower for the larger polygons. `libtess2` has the worst performance, with 283 times slower to triangulate Canada.

The `std::sort` row contains the timings of lexicographically sorting the vertices of the respective polygon, and is added for comparison. Any algorithm which requires sorting of the input vertices (such as sweep line based algorithms) won't be able to be faster than this. The fact that DidaGeom's implementation is even faster than sorting for the larger polygons just shows how fast it really is.