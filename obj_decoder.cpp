#include "obj_decoder.h"
#include "geometry.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

// TODO: std::filesystem::path

std::expected<Mesh, ParseError> read_obj(const std::filesystem::path &path,
                                         const int width, const int height) {

  const float c = sqrt(3) / 2.0f;
  const float s = 1.0f / 2;

  geo::mat3 rotationMatrix{c, 0, s, 0, 1, 0, -s, 0, c};

  std::ifstream inputFile(path);

  if (!inputFile.is_open()) {
    return std::unexpected(ParseError::InvalidFilePath);
  }

  std::vector<geo::vec3> vertices;
  std::vector<Face> faces;
  std::string line;

  while (std::getline(inputFile, line)) {
    std::istringstream iss(line);

    std::string prefix;
    iss >> prefix;

    // 1. Object Coordinates
    if (prefix == "v") {
      geo::vec3 vertex;
      iss >> vertex.x >> vertex.y >> vertex.z;
      vertices.push_back(vertex);

    } else if (prefix == "f") {
      std::string t1, t2, t3;

      if (!(iss >> t1 >> t2 >> t3))
        continue;

      int a = std::stoi(t1);
      int b = std::stoi(t2);
      int c = std::stoi(t3);

      faces.push_back({a - 1, b - 1, c - 1});
    }
  }

  // auto comp = [] (const auto& a const auto& b) -> {};
  //
  // std::ranges::sort(faces, comp);
  std::sort(
      faces.begin(), faces.end(), [&vertices](const Face &a, const Face &b) {
        float maxA =
            std::max({vertices[a.v1].z, vertices[a.v2].z, vertices[a.v3].z});
        float maxB =
            std::max({vertices[b.v1].z, vertices[b.v2].z, vertices[b.v3].z});

        return maxA < maxB;
      });

  // Find the maximum of the
  geo::vec3 minV{FLT_MAX, FLT_MAX, FLT_MAX};
  geo::vec3 maxV{-FLT_MAX, -FLT_MAX, -FLT_MAX};

  for (const auto &v : vertices) {
    minV.x = std::min(minV.x, v.x);
    minV.y = std::min(minV.y, v.y);
    minV.z = std::min(minV.z, v.z);

    maxV.x = std::max(maxV.x, v.x);
    maxV.y = std::max(maxV.y, v.y);
    maxV.z = std::max(maxV.z, v.z);
  }
  // 2. World coordinates
  geo::vec3 center = (minV + maxV) * 0.5f;
  geo::vec3 size = maxV - minV;

  float scale = 1.8f / std::max({size.x, size.y, size.z});

  for (auto &vertex : vertices) {
    vertex = (vertex - center) * scale;
  }
  std::vector<geo::vec3> projected_vertices;
  float minDEPTH = FLT_MAX;
  float maxDEPTH = -FLT_MAX;
  for (auto &vertex : vertices) {
    geo::vec3 oV;

    // 3. Eye (camera) coordinates
    vertex = rotationMatrix * vertex;

    // 4. Clip coordinates
    vertex = vertex * (1.0f / (1.0f - (vertex.z) / 2));

    // 5. Screen coordinates
    oV.x = static_cast<int>((vertex.x + 1.0f) * 0.5f * width);
    oV.y = static_cast<int>((vertex.y + 1.0f) * 0.5f * height);
    oV.z = vertex.z;
    projected_vertices.push_back(oV);
    minDEPTH = std::min(minDEPTH, oV.z);
    maxDEPTH = std::max(maxDEPTH, oV.z);
  }

  return Mesh{projected_vertices, faces, minDEPTH, maxDEPTH};
}
