#ifndef MESH_HPP
#define MESH_HPP

#include <string>
#include <vector>
#include <memory>
#include <atlas/utils/load_obj_file.hpp>
#include "objects/triangle.hpp"
#include "objects/smooth_uv_triangle.hpp"

namespace poly::object {

    class Mesh {
    public:
        std::vector<std::shared_ptr<SmoothMeshUVTriangle>> m_triangles;
        math::Vector m_position;

        Mesh(std::string const& filename, std::string const& mat_path, math::Vector position);

        void material_set(std::shared_ptr<poly::material::Material> const& material);

        void translate(math::Vector const& position);

        void scale(math::Vector const& scale);

        void fake_uvs();

        std::vector<std::shared_ptr<SmoothMeshUVTriangle>>& repr_get();

        void dump_to_list(std::vector<std::shared_ptr<Object>>& list);

    protected:
        atlas::utils::ObjMesh m_shapes;
    };
}
#endif // !MESH_HPP