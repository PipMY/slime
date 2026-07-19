#include "tgaimage.h"
#include <fstream>
#include <iostream>

TGAImage::TGAImage(const int w, const int h, const int bpp, TGAColor c)
    : w(w), h(h), bpp(bpp), data(w * h * bpp, 0) {
  for (int i = 0; i < w * h; ++i) {
    std::copy(c.bgra, c.bgra + bpp, data.begin() + i * bpp);
  }
}

bool TGAImage::read_tga_file(const std::string filename) {
  std::ifstream in(filename, std::ios::binary);
  TGAHeader header;
  in.read(reinterpret_cast<char *>(&header), sizeof(header));

  w = header.width;
  h = header.height;
  bpp = header.bitsperpixel >> 3;

  data.resize(w * h * bpp);
  in.read(reinterpret_cast<char *>(data.data()), data.size());

  if (!(header.imagedescriptor & 0x20))
    flip_vertically();
  if (header.imagedescriptor & 0x10)
    flip_horizontally();

  return true;
}

bool TGAImage::write_tga_file(const std::string filename, const bool vflip,
                              const bool rle) const {
  std::ofstream out(filename, std::ios::binary);

  TGAHeader header = {};
  header.bitsperpixel = bpp << 3;
  header.width = w;
  header.height = h;
  header.datatypecode = (bpp == GRAYSCALE ? 3 : 2);
  header.imagedescriptor = vflip ? 0x00 : 0x20;

  out.write(reinterpret_cast<const char *>(&header), sizeof(header));

  out.write(reinterpret_cast<const char *>(data.data()), data.size());

  std::uint8_t trash[8] = {0};
  out.write(reinterpret_cast<const char *>(trash), 8);
  out.write("TRUEVISION-XFILE.", 18);
  return true;
}

TGAColor TGAImage::get(const int x, const int y) const {
  if (data.empty() || x < 0 || y < 0 || x >= w || y >= h)
    return {};
  TGAColor ret = {0, 0, 0, 0, bpp};
  std::copy(data.data() + (x + y * w) * bpp,
            data.data() + (x + y * w) * bpp + bpp, ret.bgra);
  return ret;
}

void TGAImage::set(int x, int y, const TGAColor &c) {
  if (data.empty() || x < 0 || y < 0 || x >= w || y >= h)
    return;
  std::copy(c.bgra, c.bgra + bpp, data.data() + (x + y * w) * bpp);
}

void TGAImage::flip_horizontally() {
  for (int i = 0; i < w / 2; i++)
    for (int j = 0; j < h; j++)
      std::swap_ranges(data.begin() + (i + j * w) * bpp,
                       data.begin() + (i + j * w) * bpp + bpp,
                       data.begin() + (w - 1 - i + j * w) * bpp);
}

void TGAImage::flip_vertically() {
  for (int j = 0; j < h / 2; j++)
    std::swap_ranges(data.begin() + (j * w) * bpp,
                     data.begin() + (j * w + w) * bpp,
                     data.begin() + ((h - 1 - j) * w) * bpp);
}

int TGAImage::width() const { return w; }
int TGAImage::height() const { return h; }
