#include "structures/photon.hpp"

namespace poly::structures {

	Photon::Photon()
	{}

    Photon::Photon(atlas::math::Point point, atlas::math::Ray<atlas::math::Vector> ray,
        atlas::math::Normal normal, float intensity, unsigned int depth) : m_point{point}, m_wi{ray}, m_normal{normal},
        m_intensity{intensity}, m_depth{depth}
    {}

	const Photon Photon::reflect(float intensity) const
	{
		atlas::math::Vector wo = m_wi.d * 2.0f - glm::dot(m_wi.d, m_normal) * m_normal;
		atlas::math::Ray<atlas::math::Vector> photon_ray{m_point, wo};
		return Photon{m_point, photon_ray, m_normal, m_intensity * intensity, m_depth + 1};
	}

	unsigned int Photon::depth() const {
		return m_depth;
	}
}