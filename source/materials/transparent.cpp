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

	void Transparent::absorb_photon(structures::Photon &photon, std::vector<poly::structures::Photon> &photons,
									unsigned int max_depth, std::vector<std::shared_ptr<poly::object::Object>> scene) const
	{
		if (photon.depth() >= max_depth) {
			photons.push_back(photon);
			return;
		}

		float transparent_kt = m_transmitted_btdf->kt();
		float specular_kd = m_specular->kd();
		float reflective_kd = m_reflected_brdf->kd();
		float diffuse_kd = m_diffuse->kd();
		float total = transparent_kt + specular_kd + reflective_kd + diffuse_kd;

		// Random number in the range 0 to total
		float random_number = (float(rand()) / float(std::numeric_limits<int>::max())) * total;

		if (random_number < transparent_kt) {
			transmit_photon(photon, photons, max_depth, scene, photon.intensity() * transparent_kt / total);
		} else if (random_number >= transparent_kt && random_number < transparent_kt + reflective_kd) {
			bounce_photon(photon, photons, max_depth, scene, (reflective_kd + reflective_kd) / total * photon.intensity());
		}
		photon.intensity(photon.intensity() * diffuse_kd / total);
		photons.push_back(photon);
	}

	void Transparent::transmit_photon(structures::Photon &photon, std::vector<poly::structures::Photon> &photons,
									  unsigned int max_depth, std::vector<std::shared_ptr<poly::object::Object>> scene,
									  float intensity) const
	{
		if (photon.depth() >= max_depth) {
			photons.push_back(photon);
			return;
		}

		poly::structures::SurfaceInteraction si;
		si.m_normal = photon.normal();
		atlas::math::Vector wi = photon.wi().d;
		atlas::math::Vector wt;

		m_transmitted_btdf->sample_f(si, wi, wt);

		atlas::math::Ray<atlas::math::Vector> photon_ray{photon.point(), wi};

		bool is_hit{false};
		for (auto obj: scene) {
			if (obj->hit(photon_ray, si))
				is_hit = true;
		}

		if (is_hit) {
			poly::structures::Photon reflected_photon = poly::structures::Photon(photon_ray,
				si.hitpoint_get(), si.m_normal, photon.intensity() * (1 - intensity), photon.depth() + 1);
			si.m_material->absorb_photon(reflected_photon, photons, max_depth, scene);
		}
		float new_intensity = photon.intensity() * intensity;
		photon.intensity(new_intensity);
	}
} // namespace poly::material
