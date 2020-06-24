#pragma once
#ifndef MATERIAL_HPP
#define MATERIAL_HPP

namespace poly::material { class Material; }

#include "structures/surface_interaction.hpp"
#include "structures/world.hpp"

using namespace atlas;

namespace poly::material {

	using Colour = math::Vector;

    class Material {
    public:
        Material() = default;
        virtual Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const = 0;
    };
}

#endif // !MATERIAL_HPP