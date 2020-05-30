#pragma once

#include <atlas/math/math.hpp>
#include "structures/shade_rec.hpp"

namespace poly::light {

    class DirectionalLight : public Light
    {
    public:

        DirectionalLight();

        void direction_set(atlas::math::Vector const& direction);

        atlas::math::Vector direction_get([[maybe_unused]] poly::structures::ShadeRec& sr);

        Colour L(poly::structures::ShadeRec& sr);

    protected:
        atlas::math::Vector m_direction;
    };

}