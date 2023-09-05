#include "countries_geojson.hpp"
#include "dida/polygon2_utils.hpp"
#include "validation.hpp"

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "libtess2/tesselator.h"
#include "mapbox/earcut.hpp"

extern "C"
{

  using SeidelPoint = double[2];
  using SeidelTriangle = int[3];

  // Seidel's triangulate function.
  int triangulate_polygon(int ncontours, int cntr[], SeidelPoint* vertices, SeidelTriangle* triangles);
}

void benchmark_triangulate(const std::string& name, PolygonView2 polygon)
{
  std::stringstream s;
  s << name << " (" << polygon.size() << " vertices)";
  std::string name_and_num_vertices = s.str();

  /*{
    std::vector<Triangle2> triangulation = triangulate(polygon);
    CHECK(validate_triangulation(polygon, triangulation));
  }*/

  BENCHMARK(name_and_num_vertices + ", triangulate")
  {
    return triangulate(polygon);
  };

  {

    std::vector<float> vertices(2 * polygon.size());
    for (size_t i = 0; i < polygon.size(); i++)
    {
      vertices[2 * i] = static_cast<double>(polygon[i].x());
      vertices[2 * i + 1] = static_cast<double>(polygon[i].y());
    }

    // Libtess2 sometimes generates 0-area triangles, but otherwise the result seems correct.

    /*{
      TESStesselator* tessellator = tessNewTess(nullptr);
      tessAddContour(tessellator, 2, vertices.data(), 2 * sizeof(float), polygon.size());
      tessTesselate(tessellator, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, nullptr);

      size_t num_tris = tessGetElementCount(tessellator);
      const TESSindex* triangles = tessGetElements(tessellator);
      const TESSindex* index_remap = tessGetVertexIndices(tessellator);
      std::vector<Triangle2> triangulation(num_tris);
      for (size_t i = 0; i < num_tris; i++)
      {
        std::array<Point2, 3> vertices;
        for (size_t j = 0; j < 3; j++)
        {
          vertices[j] = polygon[index_remap[triangles[3 * i + j]]];
        }

        triangulation[i] = Triangle2(vertices);
      }

      tessDeleteTess(tessellator);

      CHECK(validate_triangulation(polygon, triangulation));
    }*/

    BENCHMARK(name_and_num_vertices + " libtess2")
    {
      TESStesselator* tessellator = tessNewTess(nullptr);
      tessSetOption(tessellator, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 0);
      tessAddContour(tessellator, 2, vertices.data(), 2 * sizeof(float), polygon.size());
      tessTesselate(tessellator, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, nullptr);
      tessDeleteTess(tessellator);

      return tessellator;
    };

    BENCHMARK(name_and_num_vertices + " libtess2, constrained delaunay")
    {
      TESStesselator* tessellator = tessNewTess(nullptr);
      tessSetOption(tessellator, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);
      tessAddContour(tessellator, 2, vertices.data(), 2 * sizeof(float), polygon.size());
      tessTesselate(tessellator, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, nullptr);
      tessDeleteTess(tessellator);

      return tessellator;
    };
  }

  {
    using MapboxPoint = std::pair<float, float>;
    std::vector<MapboxPoint> mapbox_ring(polygon.size());
    for (size_t i = 0; i < polygon.size(); i++)
    {
      mapbox_ring[i] = std::make_pair(static_cast<double>(polygon[i].x()), static_cast<double>(polygon[i].y()));
    }

    std::vector<std::vector<MapboxPoint>> mapbox_polygon{mapbox_ring};

    /*{
      std::vector<uint32_t> result = mapbox::earcut<uint32_t>(mapbox_polygon);
      DIDA_ASSERT(result.size() % 3 == 0);

      size_t num_tris = result.size() / 3;
      std::vector<Triangle2> triangulation(num_tris);
      for (size_t i = 0; i < num_tris; i++)
      {
        std::array<Point2, 3> vertices;
        for (size_t j = 0; j < 3; j++)
        {
          vertices[j] = polygon[result[3 * i + j]];
        }
        triangulation[i] = Triangle2(vertices);
      }

      CHECK(validate_triangulation(polygon, triangulation));
    }*/

    BENCHMARK(name_and_num_vertices + ", Mapbox earcut.hpp")
    {
      return mapbox::earcut<uint32_t>(mapbox_polygon);
    };
  }

  {
    std::vector<SeidelPoint> vertices(polygon.size() + 1);
    for (size_t i = 0; i < polygon.size(); i++)
    {
      vertices[i + 1][0] = static_cast<double>(polygon[i].x());
      vertices[i + 1][1] = static_cast<double>(polygon[i].y());
    }

    /*{
      std::vector<SeidelTriangle> result(polygon.size() - 2);

      int num_vertices = static_cast<int>(polygon.size());
      triangulate_polygon(1, &num_vertices, vertices.data(), result.data());

      std::vector<Triangle2> triangulation(result.size());
      for (size_t i = 0; i < result.size(); i++)
      {
        std::array<Point2, 3> vertices{polygon[result[i][0] - 1], polygon[result[i][1] - 1], polygon[result[i][2] - 1]};
        triangulation[i] = Triangle2(vertices);
      }

      CHECK(validate_triangulation(polygon, triangulation));
    }*/

    BENCHMARK(name_and_num_vertices + ", Seidel")
    {
      std::vector<SeidelTriangle> result(polygon.size() - 2);
      int num_vertices = static_cast<int>(polygon.size());
      triangulate_polygon(1, &num_vertices, vertices.data(), result.data());
      return result;
    };

    BENCHMARK(name_and_num_vertices + ", sort vertices lexicographically")
    {
      std::vector<Point2> result(polygon.begin(), polygon.end());
      std::sort(result.begin(), result.end(), lex_less_than);
      return result;
    };
  }
}

TEST_CASE("triangulate benchmark")
{
  CountriesGeoJson countries =
      *CountriesGeoJson::read_from_file("/home/lieven/Downloads/geo-countries_zip/archive/countries.geojson");

  benchmark_triangulate("Canada", countries.polygon_for_country("Canada"));
  benchmark_triangulate("Chile", countries.polygon_for_country("Chile"));
  benchmark_triangulate("Bangladesh", countries.polygon_for_country("Bangladesh"));
  benchmark_triangulate("Netherlands", countries.polygon_for_country("Netherlands"));
}