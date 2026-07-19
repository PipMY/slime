#include "drawing.h"
#include "geometry.h"
#include "obj_decoder.h"
#include "tgaimage.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
  constexpr int width = 1000;
  constexpr int height = 1000;
  TGAImage framebuffer(width, height, TGAImage::RGB);

  fs::path model_dir = "obj-files";
  fs::path mesh_path = model_dir / "diablo3_pose" / "diablo3_pose.obj";
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

    TGAColor rnd;
    for (int c = 0; c < 3; c++)
      rnd[c] = std::rand() % 255;
    drawing::rasterise(framebuffer, zbuffer, v0, v1, v2, mesh->minDEPTH,
                       mesh->maxDEPTH, rnd);
  }

  framebuffer.write_tga_file("framebuffer.tga");

  return 0;
} // namespace std::filesystem
