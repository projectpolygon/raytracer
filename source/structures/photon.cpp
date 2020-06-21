#include "structures/photon.hpp"

namespace poly::structures {

	Photon::Photon()
	{}

	Photon::Photon(atlas::math::Ray<atlas::math::Vector> ray, atlas::math::Point point,
				   atlas::math::Normal normal, float intensity, unsigned int depth = 0) :
				   m_wi{ray}, m_point{point}, m_normal{normal}, m_intensity{intensity}, m_depth{depth}
    {}

	const atlas::math::Ray<atlas::math::Vector> Photon::reflect_ray() const
	{
		atlas::math::Vector wo = m_wi.d * 2.0f - glm::dot(m_wi.d, m_normal) * m_normal;
		return atlas::math::Ray<atlas::math::Vector>{m_point, wo};
	}

	unsigned int Photon::depth() const
	{
		return m_depth;
	}

	float Photon::intensity() const
	{
		return m_intensity;
	}

	void Photon::intensity(const float intensity)
	{
		m_intensity = intensity;
	}
}