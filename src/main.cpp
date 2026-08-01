#include "drawing.h"
#include "geometry.h"
#include "obj_decoder.h"
#include "tgaimage.h"
#include <filesystem>
#include <iostream>
#include <limits>
#include <vector>

namespace fs = std::filesystem;

using namespace geo;

int main(int argc, char **argv) {
  constexpr int width = 1000;
  constexpr int height = 1000;
  TGAImage framebuffer(width, height, TGAImage::RGB);

  vec3 sun{1, 1, -1};

  fs::path model_dir = "assets";
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

  drawing::drawMesh(framebuffer, zbuffer, *mesh, sun);

  framebuffer.write_tga_file("framebuffer.tga");

  return 0;
}
