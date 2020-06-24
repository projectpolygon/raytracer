#include "lights/point_light.hpp"
#include "structures/world.hpp"

namespace poly::light
{

	PointLight::PointLight() : Light()
	{
		m_location = atlas::math::Vector(0.0f, 0.0f, 0.0f);
	}

	PointLight::PointLight(math::Vector const &location) : Light()
	{
		m_location = location;
	}

	void PointLight::location_set(atlas::math::Point const &location)
	{
		m_location = glm::normalize(location);
	}

	atlas::math::Vector PointLight::get_direction(poly::structures::SurfaceInteraction &sr)
	{
		math::Vector surface_point = sr.get_hitpoint();
		return glm::normalize(m_location - surface_point);
	}

	bool PointLight::in_shadow(math::Ray<math::Vector> const &shadow_ray,
								poly::structures::World const& world)
	{
		float t{std::numeric_limits<float>::max()};

		// Max distance between hitpoint and light
		math::Vector line_between = m_location - shadow_ray.o;
		float line_distance = sqrt(glm::dot(line_between, line_between));

		for (std::shared_ptr<poly::object::Object> object : world.m_scene)
		{
			// If we hit an object with distance less than max
			if (object->shadow_hit(shadow_ray, t) && t < line_distance)
			{
				return true;
			}
		}

		return false;
	}

	Colour PointLight::L(poly::structures::SurfaceInteraction &sr,
		poly::structures::World const& world)
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
			math::Vector vector = glm::normalize(m_location - sr.get_hitpoint());
			float r_squared = glm::dot(vector, vector);
			return m_colour * m_ls / (r_squared);
		}
	}

} // namespace poly::light
