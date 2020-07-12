#pragma once

#include <atlas/math/math.hpp>
#include <atlas/math/ray.hpp>
#include "objects/object.hpp"
#include "structures/bounds.hpp"
#include "structures/surface_interaction.hpp"

namespace poly::object { class Object; }

namespace poly::object {

	class Triangle : public Object {

	public:

		Triangle(std::vector<math::Vector> vertices,
			math::Vector position);

		math::Vector get_normal() const;

		float get_t(const math::Ray<math::Vector>& R) const;

		bool get_closest_intersect(math::Ray<math::Vector>const& R,
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