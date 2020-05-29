#pragma once

#include <atlas/math/math.hpp>
#include "BRDFs/BRDF.hpp"
#include "structures/shade_rec.hpp"
#include "utilities/utilities.hpp"

namespace poly::material {

    class PerfectSpecular : public BRDF {
    public:
        PerfectSpecular();

        PerfectSpecular(const float kd, Colour const& colour);

        Colour f([[maybe_unused]] poly::structures::ShadeRec const& sr,
                 [[maybe_unused]] atlas::math::Vector& w_o,
                 [[maybe_unused]] atlas::math::Vector& w_i) const;

        Colour sample_f([[maybe_unused]] poly::structures::ShadeRec const& sr,
                        [[maybe_unused]] atlas::math::Vector& w_o,
                        [[maybe_unused]] atlas::math::Vector& w_r) const;

        Colour rho([[maybe_unused]] poly::structures::ShadeRec const& sr,
                   [[maybe_unused]] atlas::math::Vector& w_o) const;

    protected:
        float m_kd;
        Colour m_cd;
    };
}