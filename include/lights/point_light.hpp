#pragma once

#include <atlas/math/math.hpp>
#include "lights/light.hpp"
#include "structures/shade_rec.hpp"

namespace poly::light {

    class PointLight : public Light
    {
    public:

        PointLight();

        PointLight(math::Vector const& location);

        void location_set(atlas::math::Point const& location);

        atlas::math::Vector direction_get(poly::structures::ShadeRec& sr);
        bool in_shadow(math::Ray<math::Vector> const& shadow_ray,
                       poly::structures::ShadeRec const& sr);

        Colour L(poly::structures::ShadeRec& sr);

    protected:
        atlas::math::Point m_location;
    };

}