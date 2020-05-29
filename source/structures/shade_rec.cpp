#pragma once

#include <memory>
#include "utilities/utilities.hpp"
#include "structures/shade_rec.hpp"
#include "structures/world.hpp"

namespace poly::structures
{
	ShadeRec::ShadeRec(World &world)
		: m_world(world), m_material{nullptr},
		  m_colour{Colour(0.0f, 0.0f, 0.0f)},
		  m_normal{math::Vector(0.0f, 0.0f, 0.0f)}
	{
		m_tmin = std::numeric_limits<float>::max();
		depth = 0;
	}

	math::Vector ShadeRec::hitpoint_get() const
	{
		return m_ray.o + m_ray.d * m_tmin;
	}
} // namespace poly::structures