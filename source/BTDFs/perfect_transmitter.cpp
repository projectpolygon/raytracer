#include "BTDFs/perfect_transmitter.hpp"

using namespace atlas;

namespace poly::material
{

	PerfectTransmitter::PerfectTransmitter() : BTDF(1.0f, 1.0f) {}
	PerfectTransmitter::PerfectTransmitter(float kt, float _ior) : BTDF(kt, _ior) {}

	bool PerfectTransmitter::tot_int_refl(poly::structures::ShadeRec const &sr) const
	{
		math::Vector w_o = -sr.m_ray.d;
		float cti = glm::dot(w_o, sr.m_normal);
		float eta = m_index_refraction;

		if (cti < 0.0f)
		{
			eta = 1.0f / eta;
		}

		return (1.0f - ((1.0f - cti * cti) / (eta * eta))) < 0.0f;
	}
	Colour PerfectTransmitter::f([[maybe_unused]] poly::structures::ShadeRec const &sr,
								 [[maybe_unused]] atlas::math::Vector &w_o,
								 [[maybe_unused]] atlas::math::Vector &w_i) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}

	Colour PerfectTransmitter::sample_f(poly::structures::ShadeRec const &sr,
										atlas::math::Vector &w_o,
										atlas::math::Vector &w_t) const
	{
		math::Vector normal = sr.m_normal;
		float cti = glm::dot(sr.m_normal, w_o);
		float eta = m_index_refraction;

		if (cti < 0.0f)
		{
			normal = -normal;
			cti = -cti;
			eta = 1.0f / eta;
		}

		float temp = 1.0f - ((1.0f - cti * cti) / (eta * eta));
		float ct2 = sqrt(temp);
		w_t = (-w_o / eta) - (ct2 - (cti / eta)) * normal;
		Colour retcol = (m_kt / (eta * eta)) * (Colour(1.0f, 1.0f, 1.0f) * (1.0f / (abs(glm::dot(sr.m_normal, w_t)))));
		return retcol;
	}

	Colour PerfectTransmitter::rho([[maybe_unused]] poly::structures::ShadeRec const &sr,
								   [[maybe_unused]] atlas::math::Vector &w_o) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}
} // namespace poly::material
