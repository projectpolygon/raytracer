#include "materials/transparent.hpp"

namespace poly::material
{

	Transparent::Transparent()
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>();
		m_transmitted_btdf = std::make_shared<PerfectTransmitter>();
	}

	Transparent::Transparent(const float amount_refl,
							 const float amount_trans,
							 float f_diffuse,
							 float f_spec,
							 Colour const &_colour,
							 float _ior,
							 float _exp)
		: Phong(f_diffuse, f_spec, _colour, _exp)
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>(amount_refl, _colour);
		m_transmitted_btdf = std::make_shared<PerfectTransmitter>(amount_trans, _ior);
	}

	Colour Transparent::shade(poly::structures::SurfaceInteraction &sr, poly::structures::World const& world) const
	{
		Colour L = Phong::shade(sr, world);
		math::Vector w_o = -sr.m_ray.d;
		math::Vector w_r;

		// Get the reflected colour and direction of the reflection
		Colour reflected_colour = m_reflected_brdf->sample_f(sr, w_o, w_r);
		math::Ray<math::Vector> reflected_ray(sr.hitpoint_get(), w_r);

		// If we have internal reflection, then the ray does not transmit
		if (m_transmitted_btdf->tot_int_refl(sr))
		{
			L += world.m_tracer->trace_ray(reflected_ray, world, sr.depth + 1);
		}
		else
		{
			L += reflected_colour * world.m_tracer->trace_ray(reflected_ray, world, sr.depth + 1) * (float)fabs(glm::dot(sr.m_normal, w_r));

			math::Vector w_t;
			Colour transmitted_colour = m_transmitted_btdf->sample_f(sr, w_o, w_t);
			math::Ray<math::Vector> transmitted_ray(sr.hitpoint_get(), w_t);
			L += transmitted_colour * world.m_tracer->trace_ray(transmitted_ray, world, sr.depth + 1) * (float)fabs(glm::dot(sr.m_normal, w_t));
		}

		return L;
	}

	void Transparent::absorb_photon([[maybe_unused]] structures::Photon &p, [[maybe_unused]] std::vector<poly::structures::Photon> &photons,
									[[maybe_unused]] unsigned int max_depth, [[maybe_unused]] std::vector<std::shared_ptr<poly::object::Object>> scene) const {

	}
} // namespace poly::material
