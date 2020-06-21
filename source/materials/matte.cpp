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

	void Matte::trace_photon(const structures::Photon &p, std::vector<poly::structures::Photon> &photons,
							 unsigned int max_depth) const {
		if (p.depth() >= max_depth) {
			photons.push_back(p);
			return;
		}

		float partition = m_diffuse->kd();
		float rgn = (float(rand()) / float(std::numeric_limits<int>::max()));
		if (rgn > partition) {
			p.reflect(partition);
		}
	}
} // namespace poly::material
