#pragma once

#include <structures/shade_rec.hpp>

namespace poly::texture {

    class Texture {
    public:
        Texture() = default;
        virtual Colour colour_get(poly::structures::ShadeRec const& sr) const = 0;
    };
}