#include "materials/phong.hpp"

namespace poly::material {

        Phong::Phong() {
          m_diffuse = std::make_shared<LambertianBRDF>(
            1.0f,
            random_colour_generate());
          m_specular = std::make_shared<GlossySpecularBRDF>(
            1.0f,
            random_colour_generate(),
            1.0f);
        }
        Phong::Phong(float f_diffuse, float f_spec, Colour c, float exp)
        {
          m_diffuse = std::make_shared<LambertianBRDF>(f_diffuse, c);
          m_specular = std::make_shared<GlossySpecularBRDF>(f_spec,c,exp);
        }

        Colour Phong::shade(poly::structures::ShadeRec& sr) const {
          // Render loop
          Colour r = Colour(0.0f, 0.0f, 0.0f);
          Colour a;
          atlas::math::Vector nullVec(0.0f, 0.0f, 0.0f);

          if (sr.m_world.m_ambient) {
            a = m_diffuse->rho(sr, nullVec)
                * sr.m_world.m_ambient->L(sr);
          }

          math::Vector w_o = -sr.m_ray.d;
          for (std::shared_ptr<poly::light::Light> light : sr.m_world.m_lights) {
            Colour L = light->L(sr);
            math::Vector w_i = light->direction_get(sr);

            float angle = glm::dot(sr.m_normal, w_i);
            if (angle >= 0) {
              r += ((m_diffuse->f(sr, nullVec, nullVec)
                     + m_specular->f(sr, w_o, w_i))
                    * L
                    * angle);
            }
            else {
              r += Colour(0.0f, 0.0f, 0.0f);
            }
          }

          return (a + r);
        }
}
