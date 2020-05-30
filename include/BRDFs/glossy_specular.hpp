#pragma once

#include "utilities/utilities.hpp"
#include "BRDFs/BRDF.hpp"


namespace poly::material {

    class GlossySpecularBRDF : public BRDF {
    public:
        GlossySpecularBRDF();

        GlossySpecularBRDF(const float kd, Colour const& colour, float exp);

        Colour f([[maybe_unused]] poly::structures::ShadeRec const& sr,
                 [[maybe_unused]] atlas::math::Vector& w_o,
                 [[maybe_unused]] atlas::math::Vector& w_i) const;

        Colour rho([[maybe_unused]] poly::structures::ShadeRec const& sr,
                   [[maybe_unused]] atlas::math::Vector& w_o) const;

    protected:
        float m_kd;
        Colour m_cd;
        float m_exp;
    };
}
