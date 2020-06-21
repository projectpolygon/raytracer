#include "materials/reflective.hpp"

namespace poly::material
{

	Reflective::Reflective()
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>();
	}

	Reflective::Reflective(const float amount_refl, float f_diffuse,
	float f_spec, Colour const &_colour, float _exp)
		: Phong(f_diffuse, f_spec, _colour, _exp)
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>(amount_refl, _colour);
	}

	Colour Reflective::shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const
	{
		Colour L = Phong::shade(sr, world);
		math::Vector w_o = -sr.m_ray.d;
		math::Vector w_r;

		// Get the reflected colour and direction of the reflection
		Colour reflected_colour = m_reflected_brdf->sample_f(sr, w_o, w_r);
		math::Ray<math::Vector> reflected_ray(sr.hitpoint_get(), w_r);

		L += reflected_colour * world.m_tracer->trace_ray(reflected_ray, world, sr.depth + 1) * (float)fabs(glm::dot(sr.m_normal, w_r));

		return L;
	}

	void Reflective::absorb_photon(structures::Photon &photon,  std::vector<poly::structures::Photon> &photons,
								   unsigned int max_depth, std::vector<std::shared_ptr<poly::object::Object>> scene) const
	{
		if (photon.depth() >= max_depth) {
			photons.push_back(photon);
			return;
		}

		float reflective_kd = m_reflected_brdf->kd();
		float diffuse_kd = m_diffuse->kd();
		float specular_kd = m_specular->kd();
		float total = reflective_kd + diffuse_kd + specular_kd;

		float rgn = (float(rand()) / float(std::numeric_limits<int>::max())) * total;

		if (rgn < reflective_kd) {
			bounce_photon(photon, photons, max_depth, scene, (photon.intensity() * reflective_kd / total));
		}
		photon.intensity(photon.intensity() * (1 - (reflective_kd / total)));
		photons.push_back(photon);
	}

} // namespace poly::material
