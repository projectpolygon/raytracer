#pragma once

#include <atlas/math/math.hpp>
#include "BRDFs/BRDF.hpp"
#include "structures/surface_interaction.hpp"
#include "utilities/utilities.hpp"

namespace poly::material {

	class PerfectSpecular : public BRDF {
	public:
		PerfectSpecular();

		PerfectSpecular(const float kd, Colour const& colour);

		Colour f([[maybe_unused]] poly::structures::SurfaceInteraction const& sr,
			[[maybe_unused]] atlas::math::Vector& w_o,
			[[maybe_unused]] atlas::math::Vector& w_i) const;

		Colour sample_f([[maybe_unused]] poly::structures::SurfaceInteraction const& sr,
			[[maybe_unused]] atlas::math::Vector& w_o,
			[[maybe_unused]] atlas::math::Vector& w_r) const;

		Colour rho([[maybe_unused]] poly::structures::SurfaceInteraction const& sr,
			[[maybe_unused]] atlas::math::Vector& w_o) const;

		float kd() const;
		Colour cd([[maybe_unused]] atlas::math::Point const& p) const;

	protected:
		float m_kd;
		Colour m_cd;
	};
}