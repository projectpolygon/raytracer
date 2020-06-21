#pragma once
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

namespace poly::material { class Material; }

#include "structures/surface_interaction.hpp"
#include "structures/world.hpp"
#include "objects/object.hpp"
#include "structures/photon.hpp"

using namespace atlas;

namespace poly::material {

    class Material {
    public:
        Material() = default;
        virtual Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const = 0;
        virtual void trace_photon(structures::Photon &photon, std::vector<poly::structures::Photon> &photons, unsigned int max_depth,
								  std::vector<std::shared_ptr<poly::object::Object>> scene) const = 0;
    };
}

#endif // !MATERIAL_HPP