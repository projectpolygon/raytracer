#include <memory>
#include "structures/surface_interaction.hpp"

namespace poly::structures
{
	SurfaceInteraction::SurfaceInteraction() :
		m_u{},
		m_v{},
		m_material{nullptr},
		m_colour{atlas::math::Vector(0.0f, 0.0f, 0.0f)},
		m_normal{}
	{
		// Initial distance to max object is m_tmin
		m_tmin = std::numeric_limits<float>::max();

		// Current recursion depth (used in reflection, refraction, and
		// recursive object deposition)
		depth = 0;
	}

	atlas::math::Vector SurfaceInteraction::get_hitpoint() const
	{
		return m_ray.o + m_ray.d * m_tmin;
	}
} // namespace poly::structures
