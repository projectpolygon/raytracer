#pragma once

#include <atlas/math/ray.hpp>
#include <atlas/math/math.hpp>
#include "utilities/utilities.hpp"

namespace poly::structures {
    class Photon {
    public:

    	Photon();

        Photon(atlas::math::Point point, atlas::math::Ray<atlas::math::Vector> ray,
			   atlas::math::Normal normal, float intensity, unsigned int depth = 0);

        const Photon reflect(float intensity) const;

        unsigned int depth() const;

    private:
        atlas::math::Point m_point;
        atlas::math::Ray<atlas::math::Vector> m_wi;
        atlas::math::Normal m_normal;
        float m_intensity;
        unsigned int m_depth;
	};
}
