#pragma once

namespace poly::object {

    class Torus : public Object {

    public:
        Torus(math::Vector center, float _a, float _b)
        {
          this->center = center;
          m_a = _a;
          m_b = _b;

          // Create the bounds
          bounds = poly::structures::Bounds3D(
            math::Vector(
              center.x - _a - _b,
              center.y - _b,
              center.z - _a - _b),
            math::Vector(
              center.x + _a + _b,
              center.y + _b,
              center.z + _a + _b));
        }

        poly::structures::Bounds3D boundbox_get() const
        {
          return bounds;
        };

        math::Vector normal_get(math::Ray<math::Vector> const& R,
                                float t) const
        {
          math::Vector point = (R.o - center) + t * R.d;
          float xyz_squared = point.x * point.x
                              + point.y * point.y
                              + point.z * point.z;
          float ab_squared = m_a * m_a
                             + m_b * m_b;
          float n_x = 4 * point.x * (xyz_squared - ab_squared);
          float n_y = 4 * point.y * (xyz_squared - ab_squared
                                     + (2 * m_a * m_a));
          float n_z = 4 * point.z * (xyz_squared - ab_squared);
          math::Vector normal = glm::normalize(math::Vector(n_x, n_y, n_z));
          return normal;
        }

        std::vector<double> rcoeffs_get(math::Ray<math::Vector>const& R) const
        {
          math::Vector O = R.o;
          math::Vector D = glm::normalize(R.d);

          // Allows us to move the torus around the screen
          O.x -= this->center.x;
          O.y -= this->center.y;
          O.z -= this->center.z;

          double sum_d_sqrd = glm::dot(D, D);
          double e = glm::dot(O, O)
                     - (m_a * (double)m_a)
                     - (m_b * (double)m_b);
          double od = glm::dot(O, D);
          double four_a_sqrd = 4.0 * m_a * m_a;

          // Coefficients
          double c_4 = sum_d_sqrd * sum_d_sqrd;

          double c_3 = 4.0 * sum_d_sqrd * od;

          double c_2 = 2.0 * sum_d_sqrd * e
                       + 4.0 * od * od
                       + four_a_sqrd * (double)D.y * D.y;

          double c_1 = 4.0 * od * e + 2.0 * four_a_sqrd * O.y * D.y;

          double c_0 = e * e
                       - four_a_sqrd * ((double)m_b * m_b - (double)O.y * O.y);

          return std::vector<double>{c_0, c_1, c_2, c_3, c_4};
        }

        bool hit(math::Ray<math::Vector>const& R,
                 poly::structures::ShadeRec& sr) const
        {
          float t{ std::numeric_limits<float>::max() };
          bool intersect = this->closest_intersect_get(R, t);

          // If this object is hit, set the shaderec with the relevant material and information about the hit point
          if (intersect && t < sr.m_tmin) {
            sr.m_normal = normal_get(R, t);
            sr.m_ray = R;
            sr.m_tmin = t;
            sr.m_material = m_material;
          }

          return intersect;
        }

        bool shadow_hit(math::Ray<math::Vector>const& R,
                        float& t) const
        {
          float temp_t;
          if (this->closest_intersect_get(R, temp_t) && temp_t < t && t > m_epsilon) {
            t = temp_t;
            return true;
          }
          else {
            return false;
          }
        }

        bool closest_intersect_get(math::Ray<math::Vector>const& R,
                                   float& t_min) const
        {
          std::vector<double> coeffs = rcoeffs_get(R);
          std::vector<double> roots;
          math::solve_quartic(coeffs, roots);

          double curr_min = DBL_MAX;
          bool intersect = false;

          for (double root : roots)
          {
            if (root < curr_min && root > 0)
            {
              curr_min = root;
              intersect = true;
            }
          }

          t_min = (float)curr_min;

          return intersect;
        }

    private:

        float m_a;
        float m_b;
        math::Vector center;

    };
}