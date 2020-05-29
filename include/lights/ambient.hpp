#pragma once

#include <atlas/math/math.hpp>
#include "lights/light.hpp"
#include "structures/shade_rec.hpp"

namespace poly::light
{

	class AmbientLight : public Light
	{
	public:
		AmbientLight();

		atlas::math::Vector direction_get([[maybe_unused]] poly::structures::ShadeRec &sr);

		Colour L([[maybe_unused]] poly::structures::ShadeRec &sr);
	};
} // namespace poly::light