#include "tracers/whitted_tracer.hpp"

using namespace atlas;

namespace poly::structures {
	WhittedTracer::WhittedTracer(poly::structures::World& world_) : Tracer(world_) { }

	Colour WhittedTracer::trace_ray(math::Ray<math::Vector> const& ray, World const& world, const unsigned int depth) const
	{
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
				Colour returned_colour = temp_sr.m_material->shade(temp_sr, world);
				return returned_colour;
			}
			else {
				return world.m_background;
			}
		}
	}
}
