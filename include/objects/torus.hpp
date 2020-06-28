#pragma once

#include <atlas/math/math.hpp>

#include "objects/object.hpp"
#include "structures/bounds.hpp"
//#include "structures/surface_interaction.hpp"

namespace poly::object { class Object; }

namespace poly::object {

	class Torus : public Object
	{

	public:
		Torus(math::Vector center, float _a, float _b);

		poly::structures::Bounds3D boundbox_get() const;

		math::Vector normal_get(math::Ray<math::Vector> const& R, float t) const;

		std::vector<double> rcoeffs_get(math::Ray<math::Vector> const& R) const;

		bool hit(math::Ray<math::Vector> const& R,
			poly::structures::SurfaceInteraction& sr) const;

		bool shadow_hit(math::Ray<math::Vector> const& R,
			float& t) const;

		bool closest_intersect_get(math::Ray<math::Vector> const& R,
			float& t_min) const;

	private:
		float m_a;
		float m_b;
		math::Vector center;
	};
} // namespace poly::object