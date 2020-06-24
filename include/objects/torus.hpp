#pragma once

#include "objects/object.hpp"
#include <atlas/math/math.hpp>
#include "structures/bounds.hpp"

namespace poly::object
{

	class Torus : public Object
	{

	public:
		Torus(math::Vector center, float _a, float _b);

		poly::structures::Bounds3D get_boundbox() const;

		math::Vector get_normal(math::Ray<math::Vector> const &R, float t) const;

		std::vector<double> get_rcoeffs(math::Ray<math::Vector> const &R) const;

		bool hit(math::Ray<math::Vector> const &R,
			poly::structures::SurfaceInteraction&sr) const;

		bool shadow_hit(math::Ray<math::Vector> const &R,
			float &t) const;

		bool get_closest_intersect(math::Ray<math::Vector> const &R,
								   float &t_min) const;

	private:
		float m_a;
		float m_b;
		math::Vector center;
	};
} // namespace poly::object