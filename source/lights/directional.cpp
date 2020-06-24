#include "lights/directional.hpp"

namespace poly::light
{

	DirectionalLight::DirectionalLight() : Light()
	{
		m_direction = atlas::math::Vector(0.0f, 0.0f, 1.0f);
	}

	void DirectionalLight::direction_set(atlas::math::Vector const &direction)
	{
		m_direction = glm::normalize(direction);
	}

	atlas::math::Vector DirectionalLight::get_direction([[maybe_unused]] poly::structures::SurfaceInteraction &sr)
	{
		return m_direction;
	}

	Colour DirectionalLight::L(poly::structures::SurfaceInteraction &sr, poly::structures::World const& world)
	{
		math::Point new_origin = sr.get_hitpoint();
		math::Vector new_direction = glm::normalize(get_direction(sr));

		math::Ray shadow_ray(new_origin + (m_surface_epsilon * new_direction),
							 new_direction);

		if (in_shadow(shadow_ray, world))
		{
			return Colour(0.0f, 0.0f, 0.0f);
		}
		else
		{
			return m_colour * m_ls;
		}
	}

} // namespace poly::light
