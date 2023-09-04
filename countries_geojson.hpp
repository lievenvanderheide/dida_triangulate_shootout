#pragma once

#include <unordered_map>
#include <string>

#include "dida/polygon2.hpp"

using namespace dida;

class CountriesGeoJson
{
public:
  static std::shared_ptr<CountriesGeoJson> read_from_file(const std::string& file_name);

  PolygonView2 polygon_for_country(const std::string& country_name) const;

private:
  CountriesGeoJson() = default;

  std::unordered_map<std::string, Polygon2> countries_;
};
