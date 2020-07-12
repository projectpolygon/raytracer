#pragma once

#include "structures/surface_interaction.hpp"

namespace poly::texture {
	class Texture {
	public:
		using Colour = atlas::math::Vector;

		Texture() = default;
		virtual Colour get_colour(poly::structures::SurfaceInteraction const& sr) const = 0;
	};
}