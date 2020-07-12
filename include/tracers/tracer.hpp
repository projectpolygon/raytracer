#pragma once
#ifndef TRACER_HPP
#define TRACER_HPP

#include <atlas/math/ray.hpp>
#include "structures/world.hpp"
#include "utilities/utilities.hpp"

using namespace atlas;

namespace poly::structures {

	class World; // avoids non-declaration in circular dependancy

	class Tracer {
	public:
		Tracer(World& world);
		virtual Colour trace_ray([[maybe_unused]] math::Ray<math::Vector> const& ray, World const& world,[[maybe_unused]] const unsigned int depth) const;

	private:
		World& m_world;
	};
}

#endif // !TRACER_HPP