#pragma once

#include <atlas/math/ray.hpp>
#include <atlas/math/math.hpp>
#include "utilities/utilities.hpp"
#include "structures/surface_interaction.hpp"

namespace poly::structures {
    class Photon {
    public:

    	Photon();

        Photon(atlas::math::Ray<atlas::math::Vector> ray, atlas::math::Point point,
			   atlas::math::Normal normal, float intensity, unsigned int depth);

		const atlas::math::Ray<atlas::math::Vector> reflect_ray() const;

		float intensity() const;

		void intensity(float intensity);

        unsigned int depth() const;

    private:
        atlas::math::Ray<atlas::math::Vector> m_wi;
		atlas::math::Point m_point;
		atlas::math::Normal m_normal;
        float m_intensity;
        unsigned int m_depth;
	};
}
