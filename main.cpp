#include "drawing.h"
#include "geometry.h"
#include "obj_decoder.h"
#include "tgaimage.h"
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <print>
#include <vector>

namespace fs = std::filesystem;

using namespace geo;

// Build a uniform grey TGAColor where
// all three RGB channels share the same intensity
TGAColor uniform_colour(std::uint8_t intensity, std::uint8_t alpha = 1) {
  TGAColor c{};
  c[0] = intensity;
  c[1] = intensity;
  c[2] = intensity;
  c[3] = alpha;
  return c;
}

// Blend three lighting layers (ambient, diffuse, specular) per-channel
// with the given weights, returning the final combined colour.
TGAColor blend(const TGAColor &ambient, const TGAColor &diffuse,
               const TGAColor &specular, float wA, float wD, float wS) {
  TGAColor out{};
  for (int i = 0; i < 3; i++) {
    out[i] = static_cast<std::uint8_t>(wA * ambient[i] + wD * diffuse[i] +
                                       wS * specular[i]);
  }
  out[3] = 1;
  return out;
}

int main(int argc, char **argv) {
  constexpr int width = 1000;
  constexpr int height = 1000;
  TGAImage framebuffer(width, height, TGAImage::RGB);

  vec3 sun{1, 1, -1};

  fs::path model_dir = "obj-files";
  fs::path mesh_path = model_dir / "african_head" / "african_head.obj";
  const auto mesh = read_obj(mesh_path, width, height);
  if (!mesh) {
    ParseError e = mesh.error();
    if (e == ParseError::InvalidFilePath)
      std::cout << "Error: Invalid file path please check.\n";
    if (e == ParseError::InvalidFileStructure)
      std::cout << "Error: invalid file structure.\n";
    return 1;
  }

  std::vector<float> zbuffer(width * height,
                             -std::numeric_limits<float>::max());

  for (auto &face : mesh->faces) {
    auto v0 = mesh->vertices[face.v1];
    auto v1 = mesh->vertices[face.v2];
    auto v2 = mesh->vertices[face.v3];

    // Ambient layer:

    TGAColor ambientColour = uniform_colour(50);

    // Diffuse layer:

    geo::vec3 mid{(v0.x + v1.x + v2.x) / 3, (v0.y + v1.y + v2.y) / 3,
                  (v0.z + v1.z + v2.z) / 3};

    geo::vec3 AB{v1 - v0};
    geo::vec3 AC{v2 - v0};

    geo::vec3 normal = geo::normalize(geo::cross(AC, AB));

    vec3 l = geo::normalize(sun);
    float diffuseIntensity = 255 * std::max(0.f, l * normal);
    TGAColor diffuseLayer =
        uniform_colour(static_cast<std::uint8_t>(diffuseIntensity));

    std::println("The l . normal: {}", l * normal);

    // Specular layer:

    vec3 eye{500, 500, 127.5};

    vec3 viewDir = geo::normalize(eye - mid);

    vec3 r = (normal * 2.0f) * (normal * l) - l;

    const float specularComponent = 2.0;
    float specularIntensity =
        255 * std::pow(std::max(0.f, r * viewDir), specularComponent);
    TGAColor specularLayer =
        uniform_colour(static_cast<std::uint8_t>(specularIntensity));

    // Final colour:
    TGAColor colour =
        blend(ambientColour, diffuseLayer, specularLayer, 0.34, 0.33, 0.33);
    drawing::rasterise(framebuffer, zbuffer, v0, v1, v2, mesh->minDEPTH,
                       mesh->maxDEPTH, colour);
  }
  geo::vec3 v0{1, 2, 3};
  geo::vec3 v1{4, 5, 6};

  auto res = geo::cross(v0, v1);

  std::println("Result of cross product: {} {} {}", res.x, res.y, res.z);

  framebuffer.write_tga_file("framebuffer.tga");

  return 0;
}
