#pragma once

#include <atlas/math/math.hpp>
#include "lights/light.hpp"
#include "structures/surface_interaction.hpp"

namespace poly::light {

	class PointLight : public Light
	{
	public:

		PointLight();

		PointLight(atlas::math::Vector const& location);

		void location_set(atlas::math::Point const& location);

		atlas::math::Vector direction_get(poly::structures::SurfaceInteraction& sr);
		bool in_shadow(atlas::math::Ray<atlas::math::Vector> const& shadow_ray,
			poly::structures::World const& world);

		Colour L(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world);

		atlas::math::Point location() const override;

	protected:
		atlas::math::Point m_location;
	};

}