#pragma once

#include <stb_image.h>
#include "mapping.hpp"

class ImageTexture : public Texture {
public:
    ImageTexture(std::string const& s) {
      int n;
      unsigned char* stbi_data = stbi_load(s.c_str(), &hres, &vres, &n, 3);

      if (stbi_data == nullptr) {
        std::cout << "ERROR: file '" << s << "' was not in the path" << std::endl;
        exit(-1);
      }

      // Create colours from the image data
      std::vector<Colour> vec_dat = std::vector<Colour>(hres * (size_t)vres);
      image_data = std::vector<std::vector<Colour>>(vres, std::vector<Colour>(hres));
      for (int i = 0; i < (int)vec_dat.size(); i++) {
        for (int j = 0; j < n; j++){
          vec_dat[i][j] = static_cast<float>(stbi_data[i * n + j]) / 255.0f;
        }
      }

      stbi_image_free(stbi_data);

      // Create a 2D vector to simplify
      for (int i = 0; i < vres; i++) {
        for (int j = 0; j < hres; j++) {
          image_data.at(vres - i - 1).at(j) = vec_dat[i * hres + j];
        }
      }
    }

    Colour colour_get([[maybe_unused]] ShadeRec const& sr) const {

      int row, col;
      if (mapper) {
        mapper->texel_coord_get(sr.hitpoint_get(), hres, vres, row, col);
      }
      else {
        row = (int)(sr.v * (vres - 1)) % (vres - 1);
        col = (int)(sr.u * (hres - 1)) % (hres - 1);
      }

      return image_data.at(row).at(col);
    }
private:
    int hres, vres;
    std::vector<std::vector<Colour>> image_data;
    std::shared_ptr<Mapping> mapper;
};