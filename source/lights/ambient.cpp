#include <atlas/math/math.hpp>
#include "lights/ambient.hpp"
#include "structures/world.hpp"

namespace poly::light {

	AmbientLight::AmbientLight() : Light() {}

	atlas::math::Vector AmbientLight::get_direction([[maybe_unused]] poly::structures::SurfaceInteraction& sr)
	{
		return atlas::math::Vector(0.0f, 0.0f, 0.0f);
	}

	Colour AmbientLight::L([[maybe_unused]] poly::structures::SurfaceInteraction& sr, [[maybe_unused]] poly::structures::World const& world)
	{
		return m_colour * m_ls;
	}

	atlas::math::Point AmbientLight::location() const
    {
	    return atlas::math::Point{0.0f, 0.0f, 0.0f};
    }
}
