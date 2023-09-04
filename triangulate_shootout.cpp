#include "countries_geojson.hpp"
#include "dida/detail/vertical_decomposition/divide_and_conquer_builder.hpp"
#include "dida/detail/vertical_decomposition/triangulate.hpp"
#include "validation.hpp"

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "libtess2/tesselator.h"
#include "mapbox/earcut.hpp"

void benchmark_triangulate(const std::string& name, PolygonView2 polygon)
{
  using namespace dida::detail::vertical_decomposition;

  {
    NodePool node_pool;
    Node* root_node = vertical_decomposition_with_divide_and_conquer_builder(
        polygon, node_pool, VerticalDecompositionType::interior_decomposition);
    std::vector<Triangle2> triangulation = triangulate(polygon, root_node);
    CHECK(validate_triangulation(polygon, triangulation));
  }

  BENCHMARK(name + ", triangulate")
  {
    NodePool node_pool;
    Node* root_node = vertical_decomposition_with_divide_and_conquer_builder(
        polygon, node_pool, VerticalDecompositionType::interior_decomposition);
    return triangulate(polygon, root_node);
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

    BENCHMARK(name + " libtess2")
    {
      TESStesselator* tessellator = tessNewTess(nullptr);
      tessSetOption(tessellator, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 0);
      tessAddContour(tessellator, 2, vertices.data(), 2 * sizeof(float), polygon.size());
      tessTesselate(tessellator, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, nullptr);
      tessDeleteTess(tessellator);

      return tessellator;
    };

    BENCHMARK(name + " libtess2, constrained delaunay")
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

    {
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
    }

    BENCHMARK(name + ", Mapbox earcut.hpp")
    {
      return mapbox::earcut<uint32_t>(mapbox_polygon);
    };
  }
}

TEST_CASE("triangulate benchmark")
{
  CountriesGeoJson countries =
      *CountriesGeoJson::read_from_file("/home/lieven/Downloads/geo-countries_zip/archive/countries.geojson");

  benchmark_triangulate("Canada", countries.polygon_for_country("Canada"));
  benchmark_triangulate("Netherlands", countries.polygon_for_country("Netherlands"));
  benchmark_triangulate("Chile", countries.polygon_for_country("Chile"));
}