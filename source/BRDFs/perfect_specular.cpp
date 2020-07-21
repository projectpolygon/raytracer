#include "BRDFs/perfect_specular.hpp"

namespace poly::material
{

	PerfectSpecular::PerfectSpecular()
	{
		m_kd = 1.0f;
		m_cd = poly::utils::random_colour_generate();
	}

	PerfectSpecular::PerfectSpecular(const float kd, Colour const &colour)
	{
		m_kd = kd;
		m_cd = colour;
	}

	Colour PerfectSpecular::f([[maybe_unused]] poly::structures::SurfaceInteraction const &sr,
							  [[maybe_unused]] atlas::math::Vector &w_o,
							  [[maybe_unused]] atlas::math::Vector &w_i) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}

	Colour PerfectSpecular::sample_f([[maybe_unused]] poly::structures::SurfaceInteraction const &sr,
									 [[maybe_unused]] atlas::math::Vector &w_o,
									 [[maybe_unused]] atlas::math::Vector &w_r) const
	{
		float ndotwo = glm::dot(sr.m_normal, w_o);
		w_r = glm::normalize(-w_o + (2.0f * sr.m_normal * ndotwo));
		return m_kd * m_cd / (float)(fabs(glm::dot(sr.m_normal, w_r)));
	}

	Colour PerfectSpecular::rho([[maybe_unused]] poly::structures::SurfaceInteraction const &sr,
								[[maybe_unused]] atlas::math::Vector &w_o) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}

	float PerfectSpecular::kd() const
	{
		return m_kd;
	}

	Colour PerfectSpecular::cd([[maybe_unused]] atlas::math::Point const& p) const
	{
		return m_cd;
	}

} // namespace poly::material
