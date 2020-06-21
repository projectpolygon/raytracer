#pragma once
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

namespace poly::material { class Material; }

#include "structures/photon.hpp"
#include "structures/surface_interaction.hpp"
#include "structures/world.hpp"
#include "objects/object.hpp"

using namespace atlas;

namespace poly::material {

    class Material {
    public:
        Material() = default;
        virtual Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const = 0;
        virtual void trace_photon(const poly::structures::Photon& p, std::vector<poly::structures::Photon>& photons,
        	unsigned int max_depth) const = 0;
    };
}

#endif // !MATERIAL_HPP