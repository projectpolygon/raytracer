#pragma once

class Triangle : public Object {

public:

    Triangle(std::vector<math::Vector> vertices,
             math::Vector position)
    {
      this->vertices = vertices;
      this->position = position;

      boundbox_calc();
    }

    math::Vector normal_get() const
    {
      math::Vector dir1 = vertices.at(0) - vertices.at(1);
      math::Vector dir2 = vertices.at(0) - vertices.at(2);

      math::Vector normal = glm::normalize(
        glm::cross(dir1, dir2));
      return normal;
    }

    float t_get(const math::Ray<math::Vector>& R) const
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
        d * (f * k - g * j)
        + b * (g * l - h * k)
        + c * (h * j - f * l);
      double gamma_num =
        a * (h * k - g * l)
        + d * (g * i - e * k)
        + c * (e * l - h * i);
      double t_num =
        a * (f * l - h * j)
        + b * (h * i - e * l)
        + d * (e * j - f * i);
      double den =
        a * (f * k - g * j)
        + b * (g * i - e * k)
        + c * (e * j - f * i);

      double beta = beta_num / den;

      if (beta < 0.0) {
        return 0;
      }

      double gamma = gamma_num / den;
      if (gamma < 0.0) {
        return 0;
      }

      if (beta + gamma > 1.0) {
        return 0;
      }

      double t = t_num / den;

      return (float)t;
    }

    bool closest_intersect_get(math::Ray<math::Vector>const& R,
                               float& t_min) const
    {
      float t = t_get(R);
      if (t > m_epsilon) {
        t_min = t;
        return true;
      }
      return false;
    }

    virtual bool hit(math::Ray<math::Vector>const& R,
                     ShadeRec& sr) const
    {
      float t{ std::numeric_limits<float>::max() };
      bool intersect = this->closest_intersect_get(R, t);

      // If this object is hit, set the shaderec with the relevant material and information about the hit point
      if (intersect && t < sr.m_tmin) {
        sr.m_normal = normal_get(); // Override
        sr.m_ray = R;
        sr.m_tmin = t;
        sr.m_material = m_material;
      }

      return intersect;
    }

    bool shadow_hit(math::Ray<math::Vector>const& R,
                    float& t) const
    {
      bool hit = this->closest_intersect_get(R, t);
      if (hit && t > m_epsilon) {
        return hit;
      }
      else {
        return false;
      }
    }

    void scale(math::Vector const& scale){
      for (size_t i = 0; i < vertices.size();i++) {
        vertices.at(i) = vertices.at(i) * scale;
      }
      boundbox_calc();
    }

    void translate(math::Vector const& pos) {
      for (size_t i = 0; i < vertices.size(); i++) {
        vertices.at(i) = vertices.at(i) + pos;
      }
      boundbox_calc();
    }

protected:

    std::vector<math::Vector> vertices;
    math::Vector position;

    void boundbox_calc() {
      float minX = vertices.at(0).x;
      float minY = vertices.at(0).y;
      float minZ = vertices.at(0).z;
      float maxX = vertices.at(0).x;
      float maxY = vertices.at(0).y;
      float maxZ = vertices.at(0).z;

      for (math::Vector vert : vertices)
      {
        vert += position;

        if (vert.x < minX) {
          minX = vert.x;
        }
        if (vert.y < minY) {
          minY = vert.y;
        }
        if (vert.z < minZ) {
          minZ = vert.z;
        }
        if (vert.x > maxX) {
          maxX = vert.x;
        }
        if (vert.y > maxY) {
          maxY = vert.y;
        }
        if (vert.z > maxZ) {
          maxZ = vert.z;
        }

      }

      bounds = Bounds3D(
        math::Vector(
          minX,
          minY,
          minZ),
        math::Vector(
          maxX,
          maxY,
          maxZ));
    }
};

class SmoothMeshUVTriangle : public Triangle {
public:
    SmoothMeshUVTriangle(std::vector<math::Vector> vertices,
                         std::vector<math::Vector2> uvs,
                         std::vector<math::Vector> normals,
                         math::Vector position) : Triangle(vertices, position) {
      m_uvs = uvs;
      m_normals = normals;
    }

    bool hit(math::Ray<math::Vector>const& R,
             ShadeRec& sr) const
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
        d * (f * k - g * j)
        + b * (g * l - h * k)
        + c * (h * j - f * l);
      float gamma_num =
        a * (h * k - g * l)
        + d * (g * i - e * k)
        + c * (e * l - h * i);
      float t_num =
        a * (f * l - h * j)
        + b * (h * i - e * l)
        + d * (e * j - f * i);
      float den =
        a * (f * k - g * j)
        + b * (g * i - e * k)
        + c * (e * j - f * i);

      float beta = beta_num / den;

      if (beta < 0.0) {
        return false;
      }

      float gamma = gamma_num / den;
      if (gamma < 0.0) {
        return false;
      }

      if (beta + gamma > 1.0f) {
        return false;
      }

      float t = t_num / den;

      if (t <= m_epsilon) {
        return false;
      }

      // If this object is hit, set the shaderec with the relevant material and information about the hit point
      if (t < sr.m_tmin) {
        sr.m_ray = R;
        sr.m_tmin = t;
        sr.m_material = m_material;
        if (m_uvs.size() == 3) {
          sr.u = interpolate_u(beta, gamma);
          sr.v = interpolate_v(beta, gamma);
        }
        if (m_normals.size() == 3) {
          sr.m_normal = interpolate_norm(beta, gamma);
        }
        else {
          sr.m_normal = normal_get(); // Override
        }
      }

      return true;
    }

    math::Vector interpolate_norm(float& beta, float& gamma) const
    {
      return (1 - beta - gamma) * m_normals.at(0) + (beta * m_normals.at(1)) + (gamma * m_normals.at(2));
    }

    float interpolate_u(float& beta, float& gamma) const
    {
      return (1 - beta - gamma) * m_uvs.at(0).x + (beta * m_uvs.at(1).x) + (gamma * m_uvs.at(2).x);
    }

    float interpolate_v(float& beta, float& gamma) const
    {
      return (1 - beta - gamma) * m_uvs.at(0).y + (beta * m_uvs.at(1).y) + (gamma * m_uvs.at(2).y);
    }

    void set_uvs(std::vector<math::Vector2> const& uvs)
    {
      if(uvs.size() == 3){
        m_uvs = uvs;
      }
    }

protected:
    std::vector<math::Vector2> m_uvs;
    std::vector<math::Vector> m_normals;
};