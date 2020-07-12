#pragma once
#ifndef WHITTEDTRACER_HPP
#define WHITTEDTRACER_HPP

#include "tracers/tracer.hpp"
#include "structures/world.hpp"
#include "structures/surface_interaction.hpp"
#include "structures/view_plane.hpp"
#include "objects/object.hpp"

namespace poly::structures {
	class WhittedTracer : public Tracer 
	{
		public:
		WhittedTracer(poly::structures::World& world);
		Colour trace_ray(math::Ray<math::Vector> const& ray, World const& world, const unsigned int depth) const;
    };
}

#endif // !WHITTEDTRACER_HPP