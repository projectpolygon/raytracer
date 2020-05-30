#pragma once

#include "BRDFs/SV_lambertian.hpp"

namespace poly::material
{

	SV_LambertianBRDF::SV_LambertianBRDF(const float kd, std::shared_ptr<poly::texture::Texture> tex)
	{
		m_kd = kd;
		m_cd = tex;
	}

	Colour SV_LambertianBRDF::f([[maybe_unused]] poly::structures::ShadeRec const &sr,
								[[maybe_unused]] atlas::math::Vector &w_o,
								[[maybe_unused]] atlas::math::Vector &w_i) const
	{
		return m_kd * m_cd->colour_get(sr) * glm::one_over_pi<float>();
	}

	Colour SV_LambertianBRDF::rho([[maybe_unused]] poly::structures::ShadeRec const &sr,
								  [[maybe_unused]] atlas::math::Vector &w_o) const
	{
		return (Colour)(m_cd->colour_get(sr) * m_kd);
	}
} // namespace poly::material
