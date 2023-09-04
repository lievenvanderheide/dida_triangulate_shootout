#pragma once

#include "dida/polygon2.hpp"
#include "dida/convex_polygon2.hpp"

using namespace dida;

/// Validates whether @c triangles form a valid triangulation of @c polygon.
///
/// The triangulation is valid if it's a tessellation of @c polygon.
bool validate_triangulation(PolygonView2 polygon, ArrayView<const Triangle2> triangles);