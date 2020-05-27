#pragma once

#include <types/atlas_types.hpp>
#include "BRDF.hpp"

namespace poly::material {

    class GlossySpecularBRDF : public BRDF {
    public:
        GlossySpecularBRDF()
        {
          m_kd = 1.0f;
          m_cd = random_colour_generate();
          m_exp = 1.0f;
        }

        GlossySpecularBRDF(const float kd, Colour const& colour, float exp)
        {
          m_kd = kd;
          m_cd = colour;
          m_exp = exp;
        }

        Colour f([[maybe_unused]] poly::structures::ShadeRec const& sr,
                 [[maybe_unused]] atlas::math::Vector& w_o,
                 [[maybe_unused]] atlas::math::Vector& w_i) const
        {
          Colour L(0.0f,0.0f,0.0f);
          float n_dot_wi = glm::dot(w_i, sr.m_normal);
          math::Vector r(-w_i + 2.0f * sr.m_normal * n_dot_wi);
          float r_dot_wo = glm::dot(w_o, r);

          if (r_dot_wo > 0.0f) {
            L = m_kd * m_cd * (float)pow(r_dot_wo, m_exp);
          }

          return L;
        }

        Colour rho([[maybe_unused]] poly::structures::ShadeRec const& sr,
                   [[maybe_unused]] atlas::math::Vector& w_o) const
        {
          return Colour(0.0f, 0.0f, 0.0f);
        }

    protected:
        float m_kd;
        Colour m_cd;
        float m_exp;
    };
}