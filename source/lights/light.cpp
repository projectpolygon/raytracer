#include "lights/light.hpp"

namespace poly::light
{

	Light::Light() : m_ls{1.0f}, m_colour{1.0f, 1.0f, 1.0f}, m_surface_epsilon{0.001f} {}

	void Light::radiance_scale(float b)
	{
		m_ls = b;
	}

	void Light::colour_set(Colour const &c)
	{
		m_colour = c;
	}
	bool Light::in_shadow(math::Ray<math::Vector> const &shadow_ray,
						  poly::structures::ShadeRec const &sr)
	{
		float t;
		for (std::shared_ptr<poly::object::Object> object : sr.m_world.m_scene)
		{
			if (object->shadow_hit(shadow_ray, t) && t > m_surface_epsilon)
			{
				return true;
			}
		}
		return false;
	}
} // namespace poly::light
