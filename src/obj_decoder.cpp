#include "obj_decoder.h"
#include "geometry.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <expected>
#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>

// --- the three camera matrices ---------------------------------------------

// World -> Eye. Builds the camera basis (l, m, n) from eye/center/up and
// folds the "-center" translation directly into the last column, so we
// never need a mat4*mat4 operator: ModelView = R * T, and since T just
// subtracts center, R*T's translation column is simply -R*center.
static geo::mat4 lookat(const geo::vec3 &eye, const geo::vec3 &center,
                        const geo::vec3 &up) {
  const geo::vec3 n = geo::normalize(
      geo::vec3{eye.x - center.x, eye.y - center.y, eye.z - center.z});
  const geo::vec3 l = geo::normalize(geo::cross(up, n));
  const geo::vec3 m = geo::normalize(geo::cross(n, l));
  return geo::mat4{
      l.x, l.y, l.z, -geo::dot(l, center), m.x, m.y, m.z, -geo::dot(m, center),
      n.x, n.y, n.z, -geo::dot(n, center), 0.f, 0.f, 0.f, 1.f};
}

// Eye -> Clip. Camera sits on the z-axis at distance f from the origin

static geo::mat4 perspective(const float f) {
  return geo::mat4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -1.0f / f, 1};
}

static geo::mat4 viewport(const int x, const int y, const int w, const int h) {
  return geo::mat4{w / 2.f,     0, 0, x + w / 2.f, 0,         h / 2.f, 0,
                   y + h / 2.f, 0, 0, 255 / 2.f,   255 / 2.f, 0,       0,
                   0,           1};
}

std::expected<Mesh, ParseError> read_obj(const std::filesystem::path &path,
                                         const int width, const int height) {
  std::ifstream inputFile(path);
  if (!inputFile.is_open()) {
    return std::unexpected(ParseError::InvalidFilePath);
  }

  std::vector<geo::vec3> vertices;
  std::vector<geo::vec3> normals;
  std::vector<geo::vec2> textures;
  std::vector<Face> faces;
  std::string line;
  while (std::getline(inputFile, line)) {

    std::replace(line.begin(), line.end(), '/', ' ');
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;
    // 1. Object coordinates
    if (prefix == "v") {
      geo::vec3 vertex;
      iss >> vertex.x >> vertex.y >> vertex.z;
      vertices.push_back(vertex);
    } else if (prefix == "vn") {
      geo::vec3 normal;
      iss >> normal.x >> normal.y >> normal.z;
      normals.push_back(normal);
    } else if (prefix == "vt") {
      geo::vec2 texture;
      iss >> texture.x >> texture.y;
      textures.push_back(texture);
      std::println("textures: {}, {}", texture.x, texture.y);
    }

    else if (prefix == "f") {
      Face face;
      iss >> face.v1 >> face.t1 >> face.n1 >> face.v2 >> face.t2 >> face.n2 >>
          face.v3 >> face.t3 >> face.n3;

      face.v1--;
      face.t1--;
      face.n1--;
      face.v2--;
      face.t2--;
      face.n2--;
      face.v3--;
      face.t3--;
      face.n3--;
      faces.push_back(face);
    }
  }

  // Bounding box, still in object coordinates
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

  // 2. World coordinates: center at origin, scale to fit [-1.8, 1.8]
  const geo::vec3 center = (minV + maxV) * 0.5f;
  const geo::vec3 size = maxV - minV;
  const float scale = 1.8f / std::max({size.x, size.y, size.z});
  for (auto &vertex : vertices) {
    vertex = (vertex - center) * scale;
  }

  // Camera setup: eye sits on the ModelView z-axis at distance
  // |eye - lookCenter| from lookCenter, which is exactly what perspective()
  // assumes as its focal distance f.
  const geo::vec3 eye{1.0f, 1.0f, 3.0f};
  const geo::vec3 lookCenter{0.0f, 0.0f, 0.0f};
  const geo::vec3 up{0.0f, 1.0f, 0.0f};
  const geo::vec3 eyeDir{eye.x - lookCenter.x, eye.y - lookCenter.y,
                         eye.z - lookCenter.z};
  const float focalLength = geo::length(eyeDir);

  const geo::mat4 ModelView = lookat(eye, lookCenter, up);
  const geo::mat4 Perspective = perspective(focalLength);
  const geo::mat4 Viewport = viewport(0, 0, width, height);

  std::vector<geo::vec3> transformed_normals;
  for (const auto &n : normals) {
    geo::vec4 normal{n.x, n.y, n.z, 0.0f};
    normal = ModelView * normal;
    transformed_normals.push_back(
        geo::normalize(geo::vec3{normal.x, normal.y, normal.z}));
  }
  std::vector<geo::vec3> projected_vertices;
  projected_vertices.reserve(vertices.size());
  float minDEPTH = FLT_MAX;
  float maxDEPTH = -FLT_MAX;

  for (const auto &vertex : vertices) {
    geo::vec4 v{vertex.x, vertex.y, vertex.z, 1.0f};

    // 3. Eye (camera) coordinates
    v = ModelView * v;

    // 4. Clip coordinates
    v = Perspective * v;

    // Perspective divide: clip -> normalized device coordinates
    v.x /= v.w;
    v.y /= v.w;
    v.z /= v.w;
    v.w = 1.0f;

    // 5. Screen coordinates
    v = Viewport * v;

    geo::vec3 oV;
    oV.x = static_cast<int>(v.x);
    oV.y = static_cast<int>(v.y);
    oV.z = static_cast<int>(v.z);

    projected_vertices.push_back(oV);
    minDEPTH = std::min(minDEPTH, oV.z);
    maxDEPTH = std::max(maxDEPTH, oV.z);
  }

  // Painter's algorithm: sort using the *projected* depth, not raw object
  // coordinates, so the ordering actually reflects what the camera sees.
  std::sort(faces.begin(), faces.end(),
            [&projected_vertices](const Face &a, const Face &b) {
              const float maxA = std::max({projected_vertices[a.v1].z,
                                           projected_vertices[a.v2].z,
                                           projected_vertices[a.v3].z});
              const float maxB = std::max({projected_vertices[b.v1].z,
                                           projected_vertices[b.v2].z,
                                           projected_vertices[b.v3].z});
              return maxA < maxB;
            });

  return Mesh{projected_vertices, transformed_normals,
              textures,           faces,
              minDEPTH,           maxDEPTH};
}
