#pragma once

#include <atlas/math/math.hpp>
#include "structures/surface_interaction.hpp"

namespace poly::material {

	class BTDF {
	public:
		BTDF(float kt, float ior)
		{
			m_kt = kt;
			m_index_refraction = ior;
		}

		virtual bool tot_int_refl(poly::structures::SurfaceInteraction const& sr) const = 0;
		virtual Colour f(poly::structures::SurfaceInteraction const& sr,
			atlas::math::Vector& w_o,
			atlas::math::Vector& w_i) const = 0;
		virtual Colour sample_f(poly::structures::SurfaceInteraction const& sr,
			atlas::math::Vector& w_o,
			atlas::math::Vector& w_t) const = 0;
		virtual Colour rho(poly::structures::SurfaceInteraction const& sr,
			atlas::math::Vector& w_o) const = 0;
	protected:
		float m_index_refraction;
		float m_kt;
	};
}