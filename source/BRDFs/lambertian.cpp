#include "BRDFs/lambertian.hpp"

using namespace atlas;

namespace poly::material
{
	LambertianBRDF::LambertianBRDF()
	{
		m_kd = 1.0f;
		m_cd = poly::utils::random_colour_generate();
	}

	LambertianBRDF::LambertianBRDF(const float kd, Colour const &colour)
	{
		m_kd = kd;
		m_cd = colour;
	}

	Colour LambertianBRDF::f(
		[[maybe_unused]] poly::structures::SurfaceInteraction const &sr,
		[[maybe_unused]] math::Vector &w_o,
		[[maybe_unused]] math::Vector &w_i) const
	{
		return m_kd * m_cd * glm::one_over_pi<float>();
	}

	Colour LambertianBRDF::rho(
		[[maybe_unused]] poly::structures::SurfaceInteraction const &sr,
		[[maybe_unused]] math::Vector &w_o) const
	{
		return (Colour)(m_cd * m_kd);
	}

	float LambertianBRDF::kd() const
	{
		return m_kd;
	}

	Colour LambertianBRDF::cd([
		[maybe_unused]] atlas::math::Point const &p) const
	{
		return m_cd;
	}
} // namespace poly::material
