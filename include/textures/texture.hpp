#pragma once

#include "structures/surface_interaction.hpp"

namespace poly::texture {
	class Texture {
	public:
		Texture() = default;
		virtual Colour colour_get(poly::structures::SurfaceInteraction const& sr) const = 0;
	};
}