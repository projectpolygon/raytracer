#pragma once

class AmbientLight : public Light {
public:
    AmbientLight() : Light() {}

    atlas::math::Vector direction_get([[maybe_unused]] ShadeRec& sr)
    {
      return atlas::math::Vector(0.0f, 0.0f, 0.0f);
    }

    Colour L([[maybe_unused]] ShadeRec& sr)
    {
      return m_colour * m_ls;
    }
};