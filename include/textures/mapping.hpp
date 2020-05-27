#pragma once

namespace poly::texture {

    class Mapping {
    public:
        Mapping() = default;
        virtual void texel_coord_get(math::Point hitpoint, int hres, int vres, int& row, int& col) const = 0;
    };
}