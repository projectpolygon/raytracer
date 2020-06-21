#include "materials/matte.hpp"

namespace poly::material
{

	Matte::Matte()
	{
		m_diffuse = std::make_shared<LambertianBRDF>(
			1.0f,
			poly::utils::random_colour_generate());
	}
	Matte::Matte(float f, Colour const &c)
	{
		m_diffuse = std::make_shared<LambertianBRDF>(f, c);
	}

	Colour Matte::shade(poly::structures::SurfaceInteraction &sr, poly::structures::World const& world) const
	{
		// Render loop
		Colour r = Colour(0.0f, 0.0f, 0.0f);
		Colour a = Colour(0.0f, 0.0f, 0.0f);
		atlas::math::Vector nullVec(0.0f, 0.0f, 0.0f);

		if (world.m_ambient)
		{
			a = m_diffuse->rho(sr, nullVec) * world.m_ambient->L(sr, world);
		}

		for (std::shared_ptr<poly::light::Light> light : world.m_lights)
		{
			Colour brdf = m_diffuse->f(sr, nullVec, nullVec);
			Colour L = light->L(sr, world);
			float angle = glm::dot(sr.m_normal, 
				light->direction_get(sr));
			if (angle > 0)
			{
				r += (brdf * L * angle);
			}
			else
			{
				r += Colour(0.0f, 0.0f, 0.0f);
			}
		}

		return (a + r);
	}

	void Matte::trace_photon(structures::Photon &photon, std::vector<poly::structures::Photon> &photons,
							 unsigned int max_depth, std::vector<std::shared_ptr<poly::object::Object>> scene) const {
		if (photon.depth() >= max_depth) {
			photons.push_back(photon);
			return;
		}

		float partition = m_diffuse->kd();
		float rgn = (float(rand()) / float(std::numeric_limits<int>::max()));
		if (rgn > partition) {
			poly::structures::SurfaceInteraction si;
			atlas::math::Ray<atlas::math::Vector> photon_ray = photon.reflect_ray();
			bool is_hit{false};

			for (auto obj: scene) {
				if (obj->hit(photon_ray, si))
					is_hit = true;
			}

			if (is_hit) {
				poly::structures::Photon reflected_photon = poly::structures::Photon(photon_ray,
					si.hitpoint_get(), si.m_normal, photon.intensity() * (1 - partition), photon.depth() + 1);
				si.m_material->trace_photon(reflected_photon, photons, max_depth, scene);
			}
			float new_intensity = photon.intensity() * partition;
			photon.intensity(new_intensity);
			photons.push_back(photon);

		}
		photons.push_back(photon);
	}
} // namespace poly::material
