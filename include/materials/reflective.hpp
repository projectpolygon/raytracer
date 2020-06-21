#pragma once
#ifndef REFLECTIVEMAT_HPP
#define REFLECTIVEMAT_HPP

#include "BRDFs/perfect_specular.hpp"
#include "materials/phong.hpp"
#include "tracers/tracer.hpp"

namespace poly::material {

    class Reflective : public Phong {
    public:
        
        Reflective();
        
        Reflective(const float amount_refl,
                   float f_diffuse,
                   float f_spec,
                   Colour const& _colour,
                   float _exp);

        Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const;

		void absorb_photon(structures::Photon &photon, std::vector<poly::structures::Photon> &photons, unsigned int max_depth,
						   std::vector<std::shared_ptr<poly::object::Object>> scene) const;

    protected:
        std::shared_ptr<PerfectSpecular> m_reflected_brdf;
    };

}

#endif // !REFLECTIVEMAT_HPP