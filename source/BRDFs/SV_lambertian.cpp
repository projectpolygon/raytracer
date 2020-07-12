#include "BRDFs/SV_lambertian.hpp"

namespace poly::material
{

	SV_LambertianBRDF::SV_LambertianBRDF(const float kd, std::shared_ptr<poly::texture::Texture> tex)
	{
		m_kd = kd;
		m_cd = tex;
	}

	Colour SV_LambertianBRDF::f([[maybe_unused]] poly::structures::SurfaceInteraction const &sr,
								[[maybe_unused]] atlas::math::Vector &w_o,
								[[maybe_unused]] atlas::math::Vector &w_i) const
	{
		return m_kd * m_cd->get_colour(sr) * glm::one_over_pi<float>();
	}

	Colour SV_LambertianBRDF::rho([[maybe_unused]] poly::structures::SurfaceInteraction const &sr,
								  [[maybe_unused]] atlas::math::Vector &w_o) const
	{
		return (Colour)(m_cd->get_colour(sr) * m_kd);
	}

	float SV_LambertianBRDF::kd() 
	{
		return m_kd;
	}

	Colour SV_LambertianBRDF::cd([[maybe_unused]] atlas::math::Point const& p) const
	{
		poly::structures::SurfaceInteraction sr;
		sr.m_ray = atlas::math::Ray < atlas::math::Vector >(p, p);
		sr.m_tmin = 1.0f;
		return m_cd->colour_get(sr);
	}
} // namespace poly::material
