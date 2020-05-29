#pragma once

#include <atlas/math/math.hpp>
#include <atlas/math/ray.hpp>
#include "objects/object.hpp"
#include "structures/bounds.hpp"
#include "structures/shade_rec.hpp"

namespace poly::object
{

	class Sphere : public Object
	{
	public:
		Sphere(math::Vector position, float radius);

		poly::structures::Bounds3D boundbox_get() const;

		math::Vector normal_get(math::Ray<math::Vector> const &R,
								float t) const;

		bool hit(math::Ray<math::Vector> const &R,
				 poly::structures::ShadeRec &sr) const;

		bool shadow_hit(math::Ray<math::Vector> const &R,
						float &t) const;

		bool closest_intersect_get(const math::Ray<math::Vector> &R,
								   float &t_min) const;

	private:
		math::Vector C; // Center of the sphere
		double r;		// Radius of the sphere
	};
} // namespace poly::object
