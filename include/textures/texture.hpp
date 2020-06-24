#pragma once

#include "structures/surface_interaction.hpp"

namespace poly::texture {
	class Texture {
	public:
		Texture() = default;
		virtual Colour get_colour(poly::structures::SurfaceInteraction const& sr) const = 0;
	};
}