#pragma once

#include "structures/shade_rec.hpp"
#include "BRDFs/BRDF.hpp"

namespace poly::material {

    class LambertianBRDF : public BRDF {
    public:
        LambertianBRDF();

        LambertianBRDF(const float kd, Colour const& colour);

        Colour f([[maybe_unused]] poly::structures::ShadeRec const& sr,
                 [[maybe_unused]] atlas::math::Vector& w_o,
                 [[maybe_unused]] atlas::math::Vector& w_i) const;

        Colour rho([[maybe_unused]] poly::structures::ShadeRec const& sr,
                   [[maybe_unused]] atlas::math::Vector& w_o) const;

    protected:
        float m_kd;
        Colour m_cd;
    };
}
