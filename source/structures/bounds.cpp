#include "structures/bounds.hpp"

namespace poly::structures
{

	Bounds3D::Bounds3D()
	{
		pMin = math::Vector(0.0f, 0.0f, 0.0f);
		pMax = math::Vector(0.0f, 0.0f, 0.0f);
	}

	Bounds3D::Bounds3D(math::Vector _pMin, math::Vector _pMax) : pMin{_pMin}, pMax{_pMax} {}

	bool Bounds3D::get_intersects(const math::Ray<math::Vector> &ray, double *hitt0, double*hitt1) const
	{
		double t0 = 0;
		double t1 = std::numeric_limits<float>::max();
		for (int i = 0; i < 3; ++i)
		{
			double invRayDir = 1 / ray.d[i];
			double tNear = (pMin[i] - ray.o[i]) * invRayDir;
			double tFar = (pMax[i] - ray.o[i]) * invRayDir;
			if (tNear > tFar)
			{
				std::swap(tNear, tFar);
			}

			tFar *= 1.0f + 2.0f * (3.0f * (double)std::numeric_limits<double>::epsilon() * 0.5f) / (1.0f - 3.0f * (double)std::numeric_limits<double>::epsilon() * 0.5f);

			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar < t1 ? tFar : t1;
			if (t0 > t1)
			{
				return false;
			}
		}
		if (hitt0)
		{
			*hitt0 = t0;
		}
		if (hitt1)
		{
			*hitt1 = t1;
		}
		return true;
	}

	math::Vector Bounds3D::diagonal() const
	{
		return pMax - pMin;
	}

	float Bounds3D::surfaceArea() const
	{
		math::Vector d = Bounds3D::diagonal();
		return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	int Bounds3D::maximum_extent() const
	{
		math::Vector d = Bounds3D::diagonal();
		if (d.x > d.y && d.x > d.z)
			return 0;
		else if (d.y > d.z)
			return 1;
		else
			return 2;
	}
} // namespace poly::structures
