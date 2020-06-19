#pragma once

#ifndef SURFACEINTERACTION_HPP
#define SURFACEINTERACTION_HPP

namespace poly::structures {
	class SurfaceInteraction;
}

#include <atlas/math/ray.hpp>
#include "materials/material.hpp"

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
		atlas::math::Vector hitpoint_get() const;
	};
}

#endif // !SURFACEINTERACTION_HPP
