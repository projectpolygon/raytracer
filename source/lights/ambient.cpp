#include "lights/ambient.hpp"
#include "utilities/utilities.hpp"

namespace poly::light {

	AmbientLight::AmbientLight() : Light() {}

	atlas::math::Vector AmbientLight::direction_get([[maybe_unused]] poly::structures::SurfaceInteraction& sr)
	{
		return atlas::math::Vector(0.0f, 0.0f, 0.0f);
	}

	Colour AmbientLight::L([[maybe_unused]] poly::structures::SurfaceInteraction& sr, [[maybe_unused]] poly::structures::World const& world)
	{
		return m_colour * m_ls;
	}
}
