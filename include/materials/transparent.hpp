#pragma once

#include <memory>
#include "BRDFs/perfect_specular.hpp"
#include "BTDFs/perfect_transmitter.hpp"
#include "materials/phong.hpp"
#include "tracers/tracer.hpp"

namespace poly::material {

    class Transparent : public Phong {
    public:
        Transparent();
        Transparent(const float amount_refl,
                    const float amount_trans,
                    float f_diffuse,
                    float f_spec,
                    Colour const& _colour,
                    float _ior,
                    float _exp);

        Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const;
    protected:
        std::shared_ptr<PerfectSpecular> m_reflected_brdf;
        std::shared_ptr<PerfectTransmitter> m_transmitted_btdf;
    };

}