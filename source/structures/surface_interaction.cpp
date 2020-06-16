#include <memory>
#include "utilities/utilities.hpp"
#include "structures/shade_rec.hpp"
#include "structures/world.hpp"

namespace poly::structures
{
	SurfaceInteraction::SurfaceInteraction()
		: m_material{ nullptr },
		m_colour{ Colour(0.0f, 0.0f, 0.0f) },
		m_normal{ math::Vector(0.0f, 0.0f, 0.0f) },
		m_u{},
		m_v{}
	{
		// Initial distance to max object is m_tmin
		m_tmin = std::numeric_limits<float>::max();

		// Current recursion depth (used in reflection, refraction, and recursive object deposition)
		depth = 0;
	}

	math::Vector SurfaceInteraction::hitpoint_get() const
	{
		return m_ray.o + m_ray.d * m_tmin;
	}
} // namespace poly::structures
