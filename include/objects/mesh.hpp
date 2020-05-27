#pragma once

namespace poly::object {

    class Mesh {
    public:
        std::vector<std::shared_ptr<SmoothMeshUVTriangle>> m_triangles;
        math::Vector m_position;

        Mesh(std::string const& filename, std::string const& mat_path, math::Vector position)
        {
          m_position = position;
          std::optional<atlas::utils::ObjMesh> opt_mesh = atlas::utils::load_obj_mesh(filename, mat_path);
          if (opt_mesh.has_value()) {
            m_shapes = opt_mesh.value();
          }
          else {
            std::cout << "ERROR: file '" << filename << "' was not in the path" << std::endl;
            exit(-1);
          }

          for (auto shape : m_shapes.shapes) {
            size_t num_indices = shape.indices.size();
            for (size_t i{}; i < num_indices;i+=3) {
              std::vector<math::Vector> vertices{};
              std::vector<math::Vector2> uvs{};
              std::vector<math::Vector> normals{};
              unsigned int j = 0;
              while (j < 3) {
                size_t index = shape.indices.at(i + j++);
                vertices.push_back(shape.vertices.at(index).position);
                if (shape.has_normals) {
                  normals.push_back(shape.vertices.at(index).normal);
                }
                if (shape.has_texture_coords) {
                  uvs.push_back(shape.vertices.at(index).tex_coord);
                }
              }
              std::shared_ptr<SmoothMeshUVTriangle> new_tri = std::make_shared<SmoothMeshUVTriangle>(vertices, uvs, normals, m_position);
              m_triangles.push_back(new_tri);
            }
          }
        }

        void material_set(std::shared_ptr<poly::material::Material> const& material)
        {
          for (auto triangle : m_triangles) {
            triangle->material_set(material);
          }
        }

        void translate(math::Vector const& position) {
          for (auto triangle : m_triangles) {
            triangle->translate(position);
          }
        }

        void scale(math::Vector const& scale) {
          for (auto triangle : m_triangles) {
            triangle->scale(scale);
          }
        }

        void fake_uvs()
        {
          std::vector<math::Vector2> fake = { math::Vector2(0.0f, 0.0f), math::Vector2(1.0f, 0.0f), math::Vector2(0.5f, 1.0f) };
          for (auto triangle : m_triangles) {
            triangle->set_uvs(fake);
          }
        }

        std::vector<std::shared_ptr<SmoothMeshUVTriangle>>& repr_get() {
          return m_triangles;
        }

        void dump_to_list(std::vector<std::shared_ptr<Object>>& list) {
          for (auto tri : m_triangles) {
            list.push_back(tri);
          }
        }

    protected:
        atlas::utils::ObjMesh m_shapes;
    };
}