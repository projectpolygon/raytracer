#include "structures/world.hpp"
#include "tracers/whitted_tracer.hpp"
#include "structures/surface_interaction.hpp"

using namespace atlas;

namespace poly::structures {
	WhittedTracer::WhittedTracer(poly::structures::World& world_) : Tracer(world_), m_world{ world_ } { }

	Colour WhittedTracer::trace_ray(math::Ray<math::Vector> const& ray, const unsigned int depth) const
	{
		World world = m_world;
		if (depth > world.m_vp->max_depth) {
			return Colour(0.0f, 0.0f, 0.0f);
		}
		else {
			SurfaceInteraction temp_sr;
			bool did_hit = false;
			for (std::shared_ptr<poly::object::Object> obj : world.m_scene) {
				if (obj->hit(ray, temp_sr)) {
					did_hit = true;
				}
			}

			// If this ray hit an object, return material's shading
			if (did_hit && temp_sr.m_material != nullptr) {
				temp_sr.depth = depth;
				temp_sr.m_ray = ray;
				return temp_sr.m_material->shade(temp_sr, world);
			}
			else {
				return world.m_background;
			}
		}
	}
}
