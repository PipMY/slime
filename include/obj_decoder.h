#pragma once

#include "geometry.h"
#include <expected>
#include <filesystem>
#include <vector>

struct Face {
  int v1, v2, v3;
};

struct Mesh {
  std::vector<geo::vec3> vertices;
  std::vector<Face> faces;
  float minDEPTH;
  float maxDEPTH;
};

enum class ParseError { InvalidFileStructure, InvalidFilePath };

std::expected<Mesh, ParseError> read_obj(const std::filesystem::path &path,
                                         const int width, const int height);
