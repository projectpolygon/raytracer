#include "structures/world.hpp"
#include "tracers/whitted_tracer.hpp"
#include "structures/shade_rec.hpp"

using namespace atlas;

namespace poly::structures {
	WhittedTracer::WhittedTracer(poly::structures::World* world) : Tracer(world) { }
	
	Colour WhittedTracer::trace_ray(math::Ray<math::Vector> const& ray, const unsigned int depth) const
	{
		if (depth > m_world->m_vp->max_depth) {
			return Colour(0.0f, 0.0f, 0.0f);
		}
		else {
			ShadeRec temp_sr = ShadeRec(*m_world);
			bool did_hit = false;
			for (std::shared_ptr<poly::object::Object> obj : m_world->m_scene) {
				if (obj->hit(ray, temp_sr)) {
					did_hit = true;
				}
			}

            // If this ray hit an object, return material's shading
			if (did_hit && temp_sr.m_material != nullptr) {
				temp_sr.depth = depth;
				temp_sr.m_ray = ray;
				return temp_sr.m_material->shade(temp_sr);
			}
			else {
				return m_world->m_background;
			}
		}
	}
}
