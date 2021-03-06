#include "materials/phong.hpp"

namespace poly::material
{
	Phong::Phong()
	{
		m_diffuse = std::make_shared<LambertianBRDF>(
			1.0f, poly::utils::random_colour_generate());
		m_specular = std::make_shared<GlossySpecularBRDF>(
			1.0f, poly::utils::random_colour_generate(), 1.0f);
		m_type = poly::structures::InteractionType::ABSORB;
	}
	Phong::Phong(float f_diffuse, float f_spec, Colour c, float exp)
	{
		m_diffuse  = std::make_shared<LambertianBRDF>(f_diffuse, c);
		m_specular = std::make_shared<GlossySpecularBRDF>(f_spec, c, exp);
		m_type	   = poly::structures::InteractionType::ABSORB;
	}

	Colour Phong::shade(poly::structures::SurfaceInteraction& sr,
						poly::structures::World const& world) const
	{
		// Render loop
		Colour r = Colour(0.0f, 0.0f, 0.0f);
		Colour a = Colour(0.0f, 0.0f, 0.0f);
		atlas::math::Vector nullVec(0.0f, 0.0f, 0.0f);

		if (world.m_ambient) {
			a = m_diffuse->rho(sr, nullVec) * world.m_ambient->L(sr, world);
		}

		math::Vector w_o = -sr.m_ray.d;
		for (std::shared_ptr<poly::light::Light> light : world.m_lights) {
			Colour L		 = light->L(sr, world);
			math::Vector w_i = light->get_direction(sr);

			float angle = glm::dot(sr.m_normal, w_i);
			if (angle >= 0) {
				r += ((m_diffuse->f(sr, nullVec, nullVec) +
					   m_specular->f(sr, w_o, w_i)) *
					  L * angle);
			}
		}

		return (a + r);
	}

	float Phong::get_diffuse_strength() const
	{
		return m_diffuse->kd();
	}
	float Phong::get_specular_strength() const
	{
		return m_specular->kd();
	}
	float Phong::get_reflective_strength() const
	{
		return 0.0f;
	}
	float Phong::get_refractive_strength() const
	{
		return 0.0f;
	}

	Colour Phong::get_hue([[maybe_unused]] atlas::math::Point const& hp) const
	{
		return m_diffuse->cd(hp);
	}

	void Phong::handle_vision_point(
		std::shared_ptr<poly::object::Object>& visible_point,
		structures::SurfaceInteraction& si,
		structures::World& world) const
	{
		(void)visible_point;
		(void)si;
		(void)world;
	}
} // namespace poly::material
