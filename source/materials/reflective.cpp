#include "materials/reflective.hpp"

namespace poly::material
{

	Reflective::Reflective()
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>();
		m_type = REFLECT;
	}

	Reflective::Reflective(const float amount_refl, float f_diffuse,
		float f_spec, Colour const& _colour, float _exp)
		: Phong(f_diffuse, f_spec, _colour, _exp)
	{
		m_reflected_brdf = std::make_shared<PerfectSpecular>(amount_refl, _colour);
		m_type = REFLECT;
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

	float Reflective::get_diffuse_strength() const
	{
		return m_diffuse->kd();
	}
	float Reflective::get_specular_strength() const
	{
		return m_specular->kd();
	}
	float Reflective::get_reflective_strength() const
	{
		return m_reflected_brdf->kd();
	}
	float Reflective::get_refractive_strength() const
	{
		return 0.0f;
	}
	/*
	void Reflective::absorb_photon(structures::Photon &photon, poly::structures::KDTree &vp_tree,
								   unsigned int max_depth, poly::structures::World& world) const
	{
		if (photon.depth() >= max_depth) {
			//photons.push_back(photon);
			return;
		}

		float reflective_kd = m_reflected_brdf->kd();
		float diffuse_kd = m_diffuse->kd();
		float specular_kd = m_specular->kd();
		float total = reflective_kd + diffuse_kd + specular_kd;

		float rgn = (float(rand()) / float(std::numeric_limits<int>::max())) * total;

		if (rgn < reflective_kd) {
			bounce_photon(photon, vp_tree, max_depth, world, (photon.intensity() * reflective_kd / total));
		}
		photon.intensity(photon.intensity() * (1 - (reflective_kd / total)));
		//photons.push_back(photon);
	}*/

	void Reflective::handle_vision_point(std::shared_ptr<poly::object::Object> &visible_point,
										 structures::SurfaceInteraction &si, structures::World &world) const
	{
		(void) visible_point;
		(void) si;
		(void) world;
	}

} // namespace poly::material
