#include "drawing.h"
#include "lighting.h"
#include "tgaimage.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace drawing {

void drawLine(TGAImage &framebuffer, int x0, int y0, int x1, int y1,
              TGAColor colour = white) {
  int deltaX = x1 - x0;
  int deltaY = y1 - y0;
  int sX = (x1 > x0) ? 1 : -1;
  int sY = (y1 > y0) ? 1 : -1;

  int dX = std::abs(deltaX);
  int dY = std::abs(deltaY);

  // if dx > dy then x is the major axis
  // else y is the major axis
  // major axis is the axis that will always increase in value each iteration

  // X is the major axis
  if (dX > dY) {
    int error = dX / 2;
    int x = x0;
    int y = y0;
    while (x != x1) {
      framebuffer.set(x, y, colour);
      x += sX;
      error -= dY;
      if (error < 0) {
        y += sY;
        error += dX;
      }
    }
  }
  // Y is the major axis
  else {
    int error = dY / 2;
    int x = x0;
    int y = y0;
    while (y != y1) {
      framebuffer.set(x, y, colour);
      y += sY;
      error -= dX;
      if (error < 0) {
        x += sX;
        error += dY;
      }
    }
    framebuffer.set(x1, y1, white);
  }
}

int inside(geo::vec3 v0, geo::vec3 v1, geo::vec3 v2) {
  return (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
}

void rasterise(TGAImage &framebuffer, std::vector<float> &zbuffer, geo::vec3 v0,
               geo::vec3 v1, geo::vec3 v2, float minDepth, float maxDepth,
               TGAColor colour) {
  int ABC = inside(v0, v1, v2);
  if (ABC == 0)
    return;

  // Finding bounding box
  // Top right point
  geo::vec3 tR = {std::max({v0.x, v1.x, v2.x}), std::max({v0.y, v1.y, v2.y})};
  // Bottom left point
  geo::vec3 bL = {std::min({v0.x, v1.x, v2.x}), std::min({v0.y, v1.y, v2.y})};

  // Clamp bounding box to screen coordinates
  bL.x = std::max(0.0f, bL.x);
  bL.y = std::max(0.0f, bL.y);
  tR.x = std::min(framebuffer.width() - 1.0f, tR.x);
  tR.y = std::min(framebuffer.height() - 1.0f, tR.y);

  geo::vec3 P = {0, 0};
  for (P.x = bL.x; P.x <= tR.x; P.x++) {
    for (P.y = bL.y; P.y <= tR.y; P.y++) {
      int ABP = inside(v0, v1, P);
      int BCP = inside(v1, v2, P);
      int CAP = inside(v2, v0, P);

      if ((ABC > 0 && ABP >= 0 && BCP >= 0 && CAP >= 0) ||
          (ABC < 0 && ABP <= 0 && BCP <= 0 && CAP <= 0)) {
        // Normalise all edge function calculations to get barycentric
        // coordinates
        float w0 = static_cast<float>(BCP) / ABC;
        float w1 = static_cast<float>(CAP) / ABC;
        float w2 = static_cast<float>(ABP) / ABC;

        // Interpolate depth for the current pixel
        float zP = w0 * v0.z + w1 * v1.z + w2 * v2.z;
        int idx = P.x + P.y * framebuffer.width();

        // Perform Z-buffer depth test
        if (zP > zbuffer[idx]) {
          zbuffer[idx] = zP;

          // Compute smooth per-pixel depth factor
          float grey = (maxDepth > minDepth)
                           ? (zP - minDepth) / (maxDepth - minDepth)
                           : 1.0f;
          TGAColor colourDepth;
          colourDepth[0] = static_cast<int>(255 * grey);
          colourDepth[1] = static_cast<int>(255 * grey);
          colourDepth[2] = static_cast<int>(255 * grey);
          colourDepth[3] = 255;

          framebuffer.set(P.x, P.y, colour);
        }
      }
    }
  }
}

void drawMesh(TGAImage &framebuffer, std::vector<float> &zbuffer,
              const Mesh &mesh, geo::vec3 sun) {
  for (auto &face : mesh.faces) {
    auto v0 = mesh.vertices[face.v1];
    auto v1 = mesh.vertices[face.v2];
    auto v2 = mesh.vertices[face.v3];

    // Ambient layer:
    TGAColor ambientColour = lighting::uniform_colour(50);

    // Diffuse layer:
    geo::vec3 mid{(v0.x + v1.x + v2.x) / 3, (v0.y + v1.y + v2.y) / 3,
                  (v0.z + v1.z + v2.z) / 3};

    geo::vec3 AB{v1 - v0};
    geo::vec3 AC{v2 - v0};

    geo::vec3 normal = geo::normalize(geo::cross(AC, AB));

    geo::vec3 l = geo::normalize(sun);
    float diffuseIntensity = 255 * std::max(0.f, l * normal);
    TGAColor diffuseLayer =
        lighting::uniform_colour(static_cast<std::uint8_t>(diffuseIntensity));

    // Specular layer:
    geo::vec3 eye{500, 500, 127.5};

    geo::vec3 viewDir = geo::normalize(eye - mid);

    geo::vec3 r = (normal * 2.0f) * (normal * l) - l;

    const float specularComponent = 2.0;
    float specularIntensity =
        255 * std::pow(std::max(0.f, r * viewDir), specularComponent);
    TGAColor specularLayer =
        lighting::uniform_colour(static_cast<std::uint8_t>(specularIntensity));

    // Final colour:
    TGAColor colour = lighting::blend(ambientColour, diffuseLayer,
                                      specularLayer, 0.34, 0.33, 0.33);
    drawing::rasterise(framebuffer, zbuffer, v0, v1, v2, mesh.minDEPTH,
                       mesh.maxDEPTH, colour);
  }
}

} // namespace drawing
