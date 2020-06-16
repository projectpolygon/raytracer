#pragma once

#include <atlas/math/math.hpp>
#include <atlas/math/ray.hpp>
#include "objects/object.hpp"
#include "structures/surface_interaction.hpp"

namespace poly::object {

	class Triangle : public Object {

	public:

		Triangle(std::vector<math::Vector> vertices,
			math::Vector position);

		math::Vector normal_get() const;

		float t_get(const math::Ray<math::Vector>& R) const;

		bool closest_intersect_get(math::Ray<math::Vector>const& R,
			float& t_min) const;

		virtual bool hit(math::Ray<math::Vector>const& R,
			poly::structures::SurfaceInteraction& sr) const;

		bool shadow_hit(math::Ray<math::Vector>const& R,
			float& t) const;

		void scale(math::Vector const& scale);

		void translate(math::Vector const& pos);

	protected:

		std::vector<math::Vector> vertices;
		math::Vector position;

		void boundbox_calc();
	};
}