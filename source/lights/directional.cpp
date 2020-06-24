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

	atlas::math::Vector DirectionalLight::direction_get([[maybe_unused]] poly::structures::SurfaceInteraction &sr)
	{
		return m_direction;
	}

	Colour DirectionalLight::L(poly::structures::SurfaceInteraction &sr, poly::structures::World const& world)
	{
		math::Point new_origin = sr.hitpoint_get();
		math::Vector new_direction = glm::normalize(direction_get(sr));

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
