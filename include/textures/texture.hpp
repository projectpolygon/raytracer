#pragma once

#include <structures/ShadeRec.hpp>

class Texture {
public:
    Texture() = default;
    virtual Colour colour_get(ShadeRec const& sr) const = 0;
};