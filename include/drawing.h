#pragma once

#include "geometry.h"
#include "obj_decoder.h"
#include "tgaimage.h"

#include <vector>

namespace drawing {
constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

void drawLine(TGAImage &framebuffer, int x0, int y0, int x1, int y1,
              TGAColor colour);

void rasterise(TGAImage &framebuffer, std::vector<float> &zbuffer, geo::vec3 v0,

               geo::vec3 v1, geo::vec3 v2, float minDepth, float maxDepth,
               TGAColor colour);

int inside(geo::vec3 v0, geo::vec3 v1, geo::vec3 v2);
} // namespace drawing
