#include "objects/smooth_uv_triangle.hpp"

namespace poly::object
{

	SmoothMeshUVTriangle::SmoothMeshUVTriangle(std::vector<math::Vector> vertices,
		std::vector<math::Vector2> uvs,
		std::vector<math::Vector> normals,
		math::Vector position) : Triangle(vertices, position)
	{
		m_uvs = uvs;
		m_normals = normals;
	}

	bool SmoothMeshUVTriangle::hit(atlas::math::Ray<math::Vector> const &R,
					 poly::structures::SurfaceInteraction &sr) const
	{
		float px = position.x;
		float py = position.y;
		float pz = position.z;

		float ax = vertices.at(0).x + px;
		float ay = vertices.at(0).y + py;
		float az = vertices.at(0).z + pz;

		float bx = vertices.at(1).x + px;
		float by = vertices.at(1).y + py;
		float bz = vertices.at(1).z + pz;

		float cx = vertices.at(2).x + px;
		float cy = vertices.at(2).y + py;
		float cz = vertices.at(2).z + pz;

		float a = ax - bx;
		float b = ax - cx;
		float c = R.d.x;

		float d = ax - R.o.x;
		float e = ay - by;
		float f = ay - cy;
		float g = R.d.y;

		float h = ay - R.o.y;
		float i = az - bz;
		float j = az - cz;
		float k = R.d.z;

		float l = az - R.o.z;

		float beta_num =
				d * (f * k - g * j) + b * (g * l - h * k) + c * (h * j - f * l);
		float gamma_num =
				a * (h * k - g * l) + d * (g * i - e * k) + c * (e * l - h * i);
		float t_num =
				a * (f * l - h * j) + b * (h * i - e * l) + d * (e * j - f * i);
		float den =
				a * (f * k - g * j) + b * (g * i - e * k) + c * (e * j - f * i);

		float beta = beta_num / den;

		if (beta < 0.0)
		{
			return false;
		}

		float gamma = gamma_num / den;
		if (gamma < 0.0)
		{
			return false;
		}

		if (beta + gamma > 1.0f)
		{
			return false;
		}

		float t = t_num / den;

		if (t <= m_epsilon)
		{
			return false;
		}

		// If this object is hit, set the SurfaceInteraction with the relevant material and information about the hit point
		if (t < sr.m_tmin)
		{
			sr.m_ray = R;
			sr.m_tmin = t;
			sr.m_material = m_material;
			if (m_uvs.size() == 3)
			{
				sr.m_u = interpolate_u(beta, gamma);
				sr.m_v = interpolate_v(beta, gamma);
			}
			if (m_normals.size() == 3)
			{
				sr.m_normal = interpolate_norm(beta, gamma);
			}
			else
			{
				sr.m_normal = get_normal(); // Override
			}
		}

		return true;
	}

	math::Vector SmoothMeshUVTriangle::interpolate_norm(float &beta, float &gamma) const
	{
		return (1 - beta - gamma) * m_normals.at(0) + (beta * m_normals.at(1)) + (gamma * m_normals.at(2));
	}

	float SmoothMeshUVTriangle::interpolate_u(float &beta, float &gamma) const
	{
		return (1 - beta - gamma) * m_uvs.at(0).x + (beta * m_uvs.at(1).x) + (gamma * m_uvs.at(2).x);
	}

	float SmoothMeshUVTriangle::interpolate_v(float &beta, float &gamma) const
	{
		return (1 - beta - gamma) * m_uvs.at(0).y + (beta * m_uvs.at(1).y) + (gamma * m_uvs.at(2).y);
	}

	void SmoothMeshUVTriangle::set_uvs(std::vector<math::Vector2> const &uvs)
	{
		if (uvs.size() == 3)
		{
			m_uvs = uvs;
		}
	}
} // namespace poly::object
