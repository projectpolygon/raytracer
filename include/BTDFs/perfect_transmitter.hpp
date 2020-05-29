#pragma once

#include <atlas/math/math.hpp>
#include "BTDFs/BTDF.hpp"
#include "structures/shade_rec.hpp"

namespace poly::material {

    class PerfectTransmitter : public BTDF {
    public:
        PerfectTransmitter();
        PerfectTransmitter(float kt, float _ior);

        bool tot_int_refl(poly::structures::ShadeRec const& sr) const;
        Colour f([[maybe_unused]] poly::structures::ShadeRec const& sr,
                 [[maybe_unused]] atlas::math::Vector& w_o,
                 [[maybe_unused]] atlas::math::Vector& w_i) const;

        Colour sample_f(poly::structures::ShadeRec const& sr,
                        atlas::math::Vector& w_o,
                        atlas::math::Vector& w_t) const;

        Colour rho([[maybe_unused]] poly::structures::ShadeRec const& sr,
                   [[maybe_unused]] atlas::math::Vector& w_o) const;
    };
}