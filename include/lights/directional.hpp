#pragma once

namespace poly::light {

    class DirectionalLight : public Light
    {
    public:

        DirectionalLight() : Light()
        {
          m_direction = atlas::math::Vector(0.0f, 0.0f, 1.0f);
        }

        void direction_set(atlas::math::Vector const& direction)
        {
          m_direction = glm::normalize(direction);
        }

        atlas::math::Vector direction_get([[maybe_unused]] poly::structures::ShadeRec& sr)
        {
          return m_direction;
        }

        Colour L(poly::structures::ShadeRec& sr)
        {
          math::Point new_origin = sr.hitpoint_get();
          math::Vector new_direction = glm::normalize(direction_get(sr));

          math::Ray shadow_ray(new_origin	+ (m_surface_epsilon* new_direction),
                               new_direction);

          if (in_shadow(shadow_ray, sr)) {
            return Colour(0.0f, 0.0f, 0.0f);
          }
          else {
            return m_colour * m_ls;
          }
        }

    protected:
        atlas::math::Vector m_direction;
    };

}