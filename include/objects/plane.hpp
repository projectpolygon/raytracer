#pragma once

#include "structures/surface_interaction.hpp"
#include "objects/object.hpp"

namespace poly::object { class Object; }
namespace poly::structures { class SurfaceInteraction; }

namespace poly::object {

	class Plane : public Object
	{

	public:
		Plane(math::Vector normal, math::Vector position);

		bool get_closest_intersect(math::Ray<math::Vector> const& R,
								   float& t_min) const;
		bool hit(math::Ray<math::Vector> const& R,
			poly::structures::SurfaceInteraction& sr) const;

		bool shadow_hit([[maybe_unused]] math::Ray<math::Vector> const& R,
			[[maybe_unused]] float& t) const;

	private:
		math::Vector normal;
		math::Vector position;
	};
} // namespace poly::object
