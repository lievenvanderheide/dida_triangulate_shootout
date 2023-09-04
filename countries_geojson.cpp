#include "countries_geojson.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

void remove_duplicates_cyclic(std::vector<Point2>& vertices)
{
  std::vector<Point2>::iterator end_it = std::unique(vertices.begin(), vertices.end());
  if (*(end_it - 1) == vertices.front())
  {
    end_it--;
  }

  vertices.erase(end_it, vertices.end());
}

std::shared_ptr<CountriesGeoJson> CountriesGeoJson::read_from_file(const std::string& file_name)
{
  std::ifstream stream(file_name);
  if (!stream)
  {
    std::cout << "Couldn't open " << file_name << std::endl;
    return nullptr;
  }

  rapidjson::Document doc;
  rapidjson::IStreamWrapper stream_wrapper(stream);
  if (doc.ParseStream(stream_wrapper).HasParseError())
  {
    std::cout << "Failed to parse " << file_name << std::endl;
    return nullptr;
  }

  std::shared_ptr<CountriesGeoJson> result(new CountriesGeoJson);

  const rapidjson::Value& features = doc["features"];
  for (size_t i = 0; i < features.Size(); i++)
  {
    const rapidjson::Value& feature = features[i];
    const rapidjson::Value& feature_properties = feature["properties"];
    const rapidjson::Value& feature_geometry = feature["geometry"];

    std::string country_name = feature_properties["ADMIN"].GetString();

    const rapidjson::Value* vertices_json;
    const rapidjson::Value& geometry_type = feature_geometry["type"];
    const rapidjson::Value& geometry_coordinates = feature_geometry["coordinates"];
    if (strcmp(geometry_type.GetString(), "Polygon") == 0)
    {
      vertices_json = &geometry_coordinates[0];
    }
    else if (strcmp(geometry_type.GetString(), "MultiPolygon") == 0)
    {
      vertices_json = &geometry_coordinates[0][0];
      for (size_t j = 1; j < geometry_coordinates.Size(); j++)
      {
        if (geometry_coordinates[j][0].Size() > vertices_json->Size())
        {
          vertices_json = &geometry_coordinates[j][0];
        }
      }
    }
    else
    {
      std::cout << "Error while parsing country " << country_name
                << ": Only Polygon and MultiPolygon are supported, but " << geometry_type.GetString() << " was found."
                << std::endl;
      continue;
    }

    std::vector<Point2> vertices(vertices_json->Size());
    for (size_t j = 0; j < vertices_json->Size(); j++)
    {
      vertices[j] = Point2((*vertices_json)[j][0].GetDouble(), (*vertices_json)[j][1].GetDouble());
    }

    remove_duplicates_cyclic(vertices);
    std::reverse(vertices.begin(), vertices.end());

    std::optional<Polygon2> polygon = Polygon2::try_construct_from_vertices(std::move(vertices));
    if (!polygon)
    {
      std::cout << "Country " << country_name << " not a valid polygon." << std::endl;
      continue;
    }

    //std::cout << country_name << ": " << polygon->size() << " vertices." << std::endl;

    result->countries_.insert(std::make_pair(country_name, *std::move(polygon)));
  }

  return result;
}

PolygonView2 CountriesGeoJson::polygon_for_country(const std::string& country_name) const
{
  std::unordered_map<std::string, Polygon2>::const_iterator it = countries_.find(country_name);
  DIDA_ASSERT(it != countries_.end());
  return it->second;
}