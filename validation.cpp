#include "validation.hpp"

#include <iostream>
#include <unordered_set>

#include "dida/predicates.hpp"
#include "dida/segment2.hpp"
#include "dida/utils.hpp"

/// Returns whether @c q is (non stricly) in front of the edge starting at @c p_edge_start with direction @c p_edge_dir.
bool is_separating_axis(Point2 p_edge_start, Vector2 p_edge_dir, const Triangle2& q)
{
  for (Point2 q_vertex : q)
  {
    if (cross(p_edge_dir, q_vertex - p_edge_start) < 0)
    {
      return false;
    }
  }

  return true;
}

/// Returns whether triangles @c a and @c b intersect.
bool intersect(const Triangle2& a, const Triangle2& b)
{
  for (size_t i = 0; i < 3; i++)
  {
    if (is_separating_axis(a[i], a[succ_modulo(i, a.size())] - a[i], b) ||
        is_separating_axis(b[i], b[succ_modulo(i, b.size())] - b[i], a))
    {
      return false;
    }
  }

  return true;
}

/// Returns whether @c a and @c b cross on their interiors.
bool interiors_cross(Segment2 a, Segment2 b)
{
  ScalarDeg2 a_start_side = cross(b.direction(), a.start() - b.start());
  ScalarDeg2 a_end_side = cross(b.direction(), a.end() - b.start());
  if ((a_start_side <= 0 && a_end_side <= 0) || (a_start_side >= 0 && a_end_side >= 0))
  {
    return false;
  }

  ScalarDeg2 b_start_side = cross(a.direction(), b.start() - a.start());
  ScalarDeg2 b_end_side = cross(a.direction(), b.end() - a.start());
  if ((b_start_side <= 0 && b_end_side <= 0) || (b_start_side >= 0 && b_end_side >= 0))
  {
    return false;
  }

  return true;
}

/// Returns whether @c triangle is contained within @c polygon.
bool is_within(PolygonView2 polygon, const Triangle2& triangle)
{
  for (size_t i = 0; i < 3; i++)
  {
    if (!is_within(polygon, triangle[i]))
    {
      return false;
    }

    Segment2 triangle_edge(triangle[i], triangle[succ_modulo<size_t>(i, 3)]);
    for (size_t j = 0; j < polygon.size(); j++)
    {
      Segment2 polygon_edge(polygon[j], polygon[succ_modulo(j, polygon.size())]);
      if (interiors_cross(triangle_edge, polygon_edge))
      {
        return false;
      }
    }
  }

  return true;
}

bool validate_triangulation(PolygonView2 polygon, ArrayView<const Triangle2> triangles)
{
  // In order to validate whether 'triangles' are a tessellation of 'polygon', we check the following:
  //
  // 1. The number of triangles is polygon.size() - 2.
  // 2. Each triangle is valid.
  // 3. All triangle vertices are vertices of 'polygon'.
  // 4. All triangles are contained within 'polygon'
  // 5. The triangles don't overlap.
  //

  if (triangles.size() != polygon.size() - 2)
  {
    std::cout << "Incorrect number of triangles in triangulation. Expected: " << polygon.size() - 2
              << ", actual: " << triangles.size() << std::endl;
    return false;
  }

  std::unordered_set<Point2> vertices_set(polygon.begin(), polygon.end());

  for (size_t i = 0; i < triangles.size(); i++)
  {
    for (size_t j = 0; j < 3; j++)
    {
      if (vertices_set.find(triangles[i][j]) == vertices_set.end())
      {
        std::cout << "triangles[" << i << "], vertex " << j << " does not occur in 'polygon'" << std::endl;
        return false;
      }
    }

    if (!validate_convex_polygon_vertices(triangles[i]))
    {
      std::cout << "triangles[" << i << "] isn't valid." << std::endl;
      return false;
    }

    if (!is_within(polygon, triangles[i]))
    {
      std::cout << "triangles[" << i << "] isn't contained within 'polygon'." << std::endl;
      return false;
    }
  }

  for (size_t i = 0; i < triangles.size(); i++)
  {
    for (size_t j = i + 1; j < triangles.size(); j++)
    {
      if (intersect(triangles[i], triangles[j]))
      {
        std::cout << "triangles[" << i << "]: " << triangles[i] << " and triangles[" << j << "] " << triangles[j]
                  << " intersect." << std::endl;
        return false;
      }
    }
  }

  return true;
}