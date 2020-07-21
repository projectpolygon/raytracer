#pragma once

#include <atlas/math/math.hpp>
#include "lights/light.hpp"
#include "structures/world.hpp"
#include "structures/surface_interaction.hpp"

namespace poly::light
{

	class AmbientLight : public Light
	{
	public:
		AmbientLight();

		atlas::math::Vector get_direction([[maybe_unused]] poly::structures::SurfaceInteraction& sr);

		Colour L([[maybe_unused]] poly::structures::SurfaceInteraction& sr, poly::structures::World const& world);

        atlas::math::Point location() const override;
	};
} // namespace poly::light