#pragma once

#include <atlas/math/math.hpp>
#include "lights/light.hpp"
#include "structures/surface_interaction.hpp"

namespace poly::light {

	class DirectionalLight : public Light
	{
	public:

		DirectionalLight();

		void direction_set(atlas::math::Vector const& direction);

		atlas::math::Vector direction_get([[maybe_unused]] poly::structures::SurfaceInteraction& sr);

		Colour L(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world);

        math::Point location() const override;

	protected:
		atlas::math::Vector m_direction;
	};

}