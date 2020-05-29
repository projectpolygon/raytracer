#pragma once
#ifndef BOUNDS_HPP
#define BOUNDS_HPP


#include <vector>
#include <atlas/math/ray.hpp>
#include "utilities/utilities.hpp"

using namespace atlas;

namespace poly::structures {
	class Bounds3D
	{
	public:
		math::Vector pMin;
		math::Vector pMax;

		Bounds3D();
		Bounds3D(math::Vector _pMin, math::Vector _pMax);

		bool get_intersects(const math::Ray<math::Vector> &ray, float *hitt0, float *hitt1) const;

		math::Vector diagonal() const;

		float surfaceArea() const;

		int maximum_extent() const;
	};
}

#endif