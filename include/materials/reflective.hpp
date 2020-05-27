#pragma once

#include <BRDFs/perfect_specular.hpp>
#include "phong.hpp"

namespace poly::material {

    class Reflective : public Phong {
    public:
        Reflective()
        {
          m_reflected_brdf = std::make_shared<PerfectSpecular>();
        }
        Reflective(const float amount_refl,
                   float f_diffuse,
                   float f_spec,
                   Colour const& _colour,
                   float _exp)
          : Phong(f_diffuse, f_spec, _colour, _exp)
        {
          m_reflected_brdf = std::make_shared<PerfectSpecular>(amount_refl, _colour);
        }
        Colour shade(poly::structures::ShadeRec& sr) const
        {
          Colour L = Phong::shade(sr);
          math::Vector w_o = -sr.m_ray.d;
          math::Vector w_r;

          // Get the reflected colour and direction of the reflection
          Colour reflected_colour = m_reflected_brdf->sample_f(sr, w_o, w_r);
          math::Ray<math::Vector> reflected_ray(sr.hitpoint_get(), w_r);

          L += reflected_colour
               * sr.m_world.m_tracer->trace_ray(reflected_ray, sr.depth + 1)
               * (float)fabs(glm::dot(sr.m_normal, w_r));

          return L;
        }
    protected:
        std::shared_ptr<PerfectSpecular> m_reflected_brdf;
    };

}