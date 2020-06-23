#pragma once

#include <atlas/math/math.hpp>
#include "utilities/utilities.hpp"
#include "structures/surface_interaction.hpp"

namespace poly::material {

	class BRDF {
	public:
		BRDF() = default;
		virtual Colour f(poly::structures::SurfaceInteraction const& sr,
			atlas::math::Vector& w_o,
			atlas::math::Vector& w_i) const = 0;
		virtual Colour rho(poly::structures::SurfaceInteraction const& sr,
			atlas::math::Vector& w_o) const = 0;
	};
}