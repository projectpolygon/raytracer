#ifndef POLY_SURFACEINTERACTION_HPP
#define POLY_SURFACEINTERACTION_HPP

#include <memory>
#include <atlas/math/ray.hpp>
#include <atlas/math/math.hpp>
#include "materials/material.hpp"

namespace poly::material { class Material; }

using Colour = atlas::math::Vector;

namespace poly::structures {
	class SurfaceInteraction {
	public:
		float m_tmin;
		float m_u, m_v;
		unsigned int depth;
		atlas::math::Ray<atlas::math::Vector> m_ray;
		std::shared_ptr<poly::material::Material> m_material;
		Colour m_colour;
		atlas::math::Normal m_normal;

		SurfaceInteraction();
		atlas::math::Vector get_hitpoint() const;
	};

	enum class InteractionType {
		ABSORB,
		REFLECT,
		TRANSMIT,
		NUM_INTERACTION_TYPES
	};

}

#endif // !SURFACEINTERACTION_HPP
