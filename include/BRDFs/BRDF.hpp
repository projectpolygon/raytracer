#pragma once

class BRDF {
public:
    BRDF() = default;
    virtual Colour f(ShadeRec const& sr,
                     atlas::math::Vector& w_o,
                     atlas::math::Vector& w_i) const = 0;
    virtual Colour rho(ShadeRec const& sr,
                       atlas::math::Vector& w_o) const = 0;
};