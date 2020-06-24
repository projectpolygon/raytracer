#pragma once
#ifndef WHITTEDTRACER_HPP
#define WHITTEDTRACER_HPP

#include "tracers/tracer.hpp"
#include "structures/world.hpp"
#include "objects/object.hpp"
#include "structures/view_plane.hpp"

namespace poly::structures {
	class WhittedTracer : public Tracer 
	{
		public:
		WhittedTracer(poly::structures::World& world);
		Colour trace_ray(math::Ray<math::Vector> const& ray, const unsigned int depth) const;
		World m_world;
    };
}

#endif // !WHITTEDTRACER_HPP