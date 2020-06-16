#pragma once

#include "structures/surface_interaction.hpp"
#include "BRDFs/BRDF.hpp"

namespace poly::material {

	class LambertianBRDF : public BRDF {
	public:
		LambertianBRDF();

		LambertianBRDF(const float kd, Colour const& colour);

		Colour f([[maybe_unused]] poly::structures::SurfaceInteraction const& sr,
			[[maybe_unused]] atlas::math::Vector& w_o,
			[[maybe_unused]] atlas::math::Vector& w_i) const;

		Colour rho([[maybe_unused]] poly::structures::SurfaceInteraction const& sr,
			[[maybe_unused]] atlas::math::Vector& w_o) const;

	protected:
		float m_kd;
		Colour m_cd;
	};
}
