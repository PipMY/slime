#pragma once

#include "tgaimage.h"
#include "geometry.h"

namespace lighting {

TGAColor uniform_colour(std::uint8_t intensity, std::uint8_t alpha = 1);

TGAColor blend(const TGAColor &ambient, const TGAColor &diffuse,
               const TGAColor &specular, float wA, float wD, float wS);

TGAColor compute_lighting(geo::vec3 vertex, geo::vec3 normal, geo::vec3 sun,
                          geo::vec3 eye, TGAColor baseColor);

} // namespace lighting
