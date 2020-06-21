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
		constexpr float e = 0.001f;
		atlas::math::Vector wo = m_wi.d * 2.0f - glm::dot(m_wi.d, m_normal) * m_normal;
		return atlas::math::Ray<atlas::math::Vector>{m_point + (wo * e), wo};
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

	const atlas::math::Normal Photon::normal() const
	{
		return m_normal;
	}

	const atlas::math::Ray<atlas::math::Vector> Photon::wi() const
	{
		return m_wi;
	}

	const atlas::math::Point Photon::point() const
	{
		return m_point;
	}
}