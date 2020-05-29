#pragma once
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "structures/shade_rec.hpp"

namespace poly::structures { class ShadeRec; }

using namespace atlas;

namespace poly::material {

    class Material {
    public:
        Material() = default;
        virtual Colour shade(poly::structures::ShadeRec& sr) const = 0;
    };
}

#endif // !MATERIAL_HPP