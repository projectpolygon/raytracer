#pragma once

#include <vector>
#include <atlas/math/math.hpp>
#include "objects/triangle.hpp"

namespace poly::object {

    class SmoothMeshUVTriangle : public Triangle {
    public:
        SmoothMeshUVTriangle(std::vector<math::Vector> vertices,
                             std::vector<math::Vector2> uvs,
                             std::vector<math::Vector> normals,
                             math::Vector position);

        bool hit(atlas::math::Ray<math::Vector>const& R,
                 poly::structures::SurfaceInteraction& sr) const;

        math::Vector interpolate_norm(float& beta, float& gamma) const;

        float interpolate_u(float& beta, float& gamma) const;

        float interpolate_v(float& beta, float& gamma) const;

        void set_uvs(std::vector<math::Vector2> const& uvs);

    protected:
        std::vector<math::Vector2> m_uvs;
        std::vector<math::Vector> m_normals;
    };
}