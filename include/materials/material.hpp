#pragma once

#include <structures/ShadeRec.hpp>

class Material {
public:
    Material() = default;
    virtual Colour shade(ShadeRec& sr) const = 0;
};