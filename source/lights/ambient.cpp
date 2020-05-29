#pragma once

#include "lights/ambient.hpp"
#include "utilities/utilities.hpp"

namespace poly::light {

        AmbientLight::AmbientLight() : Light() {}

        atlas::math::Vector AmbientLight::direction_get([[maybe_unused]] poly::structures::ShadeRec& sr)
        {
          return atlas::math::Vector(0.0f, 0.0f, 0.0f);
        }

        Colour AmbientLight::L([[maybe_unused]] poly::structures::ShadeRec& sr)
        {
          return m_colour * m_ls;
        }
}