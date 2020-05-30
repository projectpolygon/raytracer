#pragma once

#include <atlas/math/math.hpp>
#include "BRDFs/BRDF.hpp"
#include "structures/shade_rec.hpp"
#include "textures/texture.hpp"


namespace poly::material {

    class SV_LambertianBRDF : public BRDF{
    public:
        SV_LambertianBRDF(const float kd, std::shared_ptr<poly::texture::Texture> tex);

        Colour f([[maybe_unused]] poly::structures::ShadeRec const& sr,
                 [[maybe_unused]] atlas::math::Vector& w_o,
                 [[maybe_unused]] atlas::math::Vector& w_i) const;

        Colour rho([[maybe_unused]] poly::structures::ShadeRec const& sr,
                   [[maybe_unused]] atlas::math::Vector& w_o) const;

    protected:
        float m_kd;
        std::shared_ptr<poly::texture::Texture> m_cd;
    };
}
