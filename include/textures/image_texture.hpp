#pragma once

#include <vector>
#include <string>
#include <memory>
#include <stb_image.h>
#include "mapping.hpp"
#include "utilities/utilities.hpp"
#include "textures/texture.hpp"

namespace poly::texture {

    class ImageTexture : public Texture {
    public:
        ImageTexture(std::string const& s);
        Colour colour_get([[maybe_unused]] poly::structures::ShadeRec const& sr) const;
    private:
        int hres, vres;
        std::vector<std::vector<Colour>> image_data;
        std::shared_ptr<Mapping> mapper;
    };
}