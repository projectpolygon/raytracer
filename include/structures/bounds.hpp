#pragma once
#ifndef BOUNDS_HPP
#define BOUNDS_HPP


#include <vector>
#include <atlas/math/ray.hpp>
#include <atlas/math/math.hpp>

using namespace atlas;

namespace poly::structures {
	class Bounds3D
	{
	public:
		atlas::math::Vector pMin;
		math::Vector pMax;

		Bounds3D();
		Bounds3D(atlas::math::Vector _pMin, math::Vector _pMax);
		bool inside_bounds(atlas::math::Point const& point, float max_dist_to_check = 0.0f) const;
		bool get_intersects(const atlas::math::Ray<atlas::math::Vector> &ray, double*hitt0, double*hitt1) const;

		atlas::math::Vector diagonal() const;

		float surfaceArea() const;

		int maximum_extent() const;
	};
}

#endif