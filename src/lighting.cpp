#include "lighting.h"
#include <algorithm>
#include <cmath>

namespace lighting {

// Build a uniform grey TGAColor where
// all three RGB channels share the same intensity
TGAColor uniform_colour(std::uint8_t intensity, std::uint8_t alpha) {
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

TGAColor compute_lighting(geo::vec3 vertex, geo::vec3 normal, geo::vec3 sun,
                          geo::vec3 eye, TGAColor baseColor) {
  // Diffuse layer
  geo::vec3 l = geo::normalize(sun);
  float diffuseIntensity = std::max(0.f, geo::dot(l, normal));
  TGAColor diffuseLayer = baseColor;
  diffuseLayer[0] = static_cast<std::uint8_t>(diffuseLayer[0] * diffuseIntensity);
  diffuseLayer[1] = static_cast<std::uint8_t>(diffuseLayer[1] * diffuseIntensity);
  diffuseLayer[2] = static_cast<std::uint8_t>(diffuseLayer[2] * diffuseIntensity);

  // Specular layer
  geo::vec3 viewDir = geo::normalize(eye - vertex);
  geo::vec3 r = (normal * 2.0f) * geo::dot(normal, l) - l;

  const float specularComponent = 2.0;
  float specularIntensity =
      255 * std::pow(std::max(0.f, geo::dot(r, viewDir)), specularComponent);
  TGAColor specularLayer =
      uniform_colour(static_cast<std::uint8_t>(specularIntensity));

  return blend(baseColor, diffuseLayer, specularLayer, 0.2, 0.8, 0.3);
}

} // namespace lighting
