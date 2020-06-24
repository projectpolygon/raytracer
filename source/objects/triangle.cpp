#include "objects/triangle.hpp"

namespace poly::object
{

	Triangle::Triangle(std::vector<math::Vector> vertices,
					   math::Vector position)
	{
		this->vertices = vertices;
		this->position = position;

		boundbox_calc();
	}

	math::Vector Triangle::get_normal() const
	{
		math::Vector dir1 = vertices.at(0) - vertices.at(1);
		math::Vector dir2 = vertices.at(0) - vertices.at(2);

		math::Vector normal = glm::normalize(
			glm::cross(dir1, dir2));
		return normal;
	}

	float Triangle::get_t(const math::Ray<math::Vector> &R) const
	{
		double px = position.x;
		double py = position.y;
		double pz = position.z;

		double ax = vertices.at(0).x + px;
		double ay = vertices.at(0).y + py;
		double az = vertices.at(0).z + pz;

		double bx = vertices.at(1).x + px;
		double by = vertices.at(1).y + py;
		double bz = vertices.at(1).z + pz;

		double cx = vertices.at(2).x + px;
		double cy = vertices.at(2).y + py;
		double cz = vertices.at(2).z + pz;

		double a = ax - bx;
		double b = ax - cx;
		double c = R.d.x;

		double d = ax - R.o.x;
		double e = ay - by;
		double f = ay - cy;
		double g = R.d.y;

		double h = ay - R.o.y;
		double i = az - bz;
		double j = az - cz;
		double k = R.d.z;

		double l = az - R.o.z;

		double beta_num =
			d * (f * k - g * j) + b * (g * l - h * k) + c * (h * j - f * l);
		double gamma_num =
			a * (h * k - g * l) + d * (g * i - e * k) + c * (e * l - h * i);
		double t_num =
			a * (f * l - h * j) + b * (h * i - e * l) + d * (e * j - f * i);
		double den =
			a * (f * k - g * j) + b * (g * i - e * k) + c * (e * j - f * i);

		double beta = beta_num / den;

		if (beta < 0.0)
		{
			return 0;
		}

		double gamma = gamma_num / den;
		if (gamma < 0.0)
		{
			return 0;
		}

		if (beta + gamma > 1.0)
		{
			return 0;
		}

		double t = t_num / den;

		return (float)t;
	}

	bool Triangle::get_closest_intersect(math::Ray<math::Vector> const &R,
										 float &t_min) const
	{
		float t = get_t(R);
		if (t > m_epsilon)
		{
			t_min = t;
			return true;
		}
		return false;
	}

	bool Triangle::hit(math::Ray<math::Vector> const &R,
							   poly::structures::SurfaceInteraction &sr) const
	{
		float t{std::numeric_limits<float>::max()};
		bool intersect = this->get_closest_intersect(R, t);

		// If this object is hit, set the SurfaceInteraction with the relevant material and information about the hit point
		if (intersect && t < sr.m_tmin)
		{
			sr.m_normal = get_normal(); // Override
			sr.m_ray = R;
			sr.m_tmin = t;
			sr.m_material = m_material;
		}

		return intersect;
	}

	bool Triangle::shadow_hit(math::Ray<math::Vector> const &R,
							  float &t) const
	{
		bool hit = this->get_closest_intersect(R, t);
		if (hit && t > m_epsilon)
		{
			return hit;
		}
		else
		{
			return false;
		}
	}

	void Triangle::scale(math::Vector const &scale)
	{
		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices.at(i) = vertices.at(i) * scale;
		}
		boundbox_calc();
	}

	void Triangle::translate(math::Vector const &pos)
	{
		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices.at(i) = vertices.at(i) + pos;
		}
		boundbox_calc();
	}

	void Triangle::boundbox_calc()
	{
		float minX = vertices.at(0).x;
		float minY = vertices.at(0).y;
		float minZ = vertices.at(0).z;
		float maxX = vertices.at(0).x;
		float maxY = vertices.at(0).y;
		float maxZ = vertices.at(0).z;

		for (math::Vector vert : vertices)
		{
			vert += position;

			if (vert.x < minX)
			{
				minX = vert.x;
			}
			if (vert.y < minY)
			{
				minY = vert.y;
			}
			if (vert.z < minZ)
			{
				minZ = vert.z;
			}
			if (vert.x > maxX)
			{
				maxX = vert.x;
			}
			if (vert.y > maxY)
			{
				maxY = vert.y;
			}
			if (vert.z > maxZ)
			{
				maxZ = vert.z;
			}
		}

		bounds = poly::structures::Bounds3D(
			math::Vector(
				minX,
				minY,
				minZ),
			math::Vector(
				maxX,
				maxY,
				maxZ));
	}
}
