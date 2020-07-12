#pragma once
#ifndef MAPPING_HPP
#define MAPPING_HPP

#include <atlas/math/math.hpp>

namespace poly::texture {

    class Mapping {
    public:
        virtual void get_texel_coord(atlas::math::Point hitpoint, int hres, int vres, int& row, int& col) const = 0;
    };
}
#endif // !MAPPING_HPP