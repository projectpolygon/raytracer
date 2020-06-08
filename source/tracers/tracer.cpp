#include "structures/world.hpp"
#include "tracers/tracer.hpp"

namespace poly::structures {
	Tracer::Tracer(poly::structures::World* _world): m_world{_world} {}
	
	Colour Tracer::trace_ray([[maybe_unused]]math::Ray<math::Vector> const& ray, [[maybe_unused]] const unsigned int depth) const
	{
		return Colour(0.0f, 0.0f, 0.0f);
	}
}
