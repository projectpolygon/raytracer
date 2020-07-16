#include "structures/KDTree.hpp"
#include "objects/mesh.hpp"
#include <iostream>

namespace poly::object
{
	Mesh::Mesh(std::string const& filename,
			   std::string const& mat_path,
			   atlas::math::Vector position)
	{
		m_position = position;
		std::optional<atlas::utils::ObjMesh> opt_mesh =
			atlas::utils::load_obj_mesh(filename, mat_path);
		if (opt_mesh.has_value()) {
			m_shapes = opt_mesh.value();
		}
		else {
			std::cout << "ERROR: file '" << filename << "' was not in the path"
					  << std::endl;
			exit(-1);
		}

		for (auto shape : m_shapes.shapes) {
			std::size_t num_indices = shape.indices.size();
			for (std::size_t i{}; i < num_indices; i += 3) {
				std::vector<atlas::math::Vector> vertices{};
				std::vector<atlas::math::Vector2> uvs{};
				std::vector<atlas::math::Vector> normals{};
				unsigned int j = 0;
				while (j < 3) {
					std::size_t index = shape.indices.at(i + j++);
					vertices.push_back(shape.vertices.at(index).position);
					if (shape.has_normals) {
						normals.push_back(shape.vertices.at(index).normal);
					}
					if (shape.has_texture_coords) {
						uvs.push_back(shape.vertices.at(index).tex_coord);
					}
				}
				std::shared_ptr<SmoothMeshUVTriangle> new_tri =
					std::make_shared<SmoothMeshUVTriangle>(
						vertices, uvs, normals, m_position);
				m_triangles.push_back(new_tri);
			}
		}
	}

	void Mesh::material_set(
		std::shared_ptr<poly::material::Material> const& material)
	{
		for (auto triangle : m_triangles) {
			triangle->material_set(material);
		}
	}

	void Mesh::translate(atlas::math::Vector const& position)
	{
		for (auto triangle : m_triangles) {
			triangle->translate(position);
		}
	}

	void Mesh::scale(atlas::math::Vector const& scale)
	{
		for (auto triangle : m_triangles) {
			triangle->scale(scale);
		}
	}

	void Mesh::fake_uvs()
	{
		std::vector<atlas::math::Vector2> fake = {
			atlas::math::Vector2(0.0f, 0.0f),
			atlas::math::Vector2(1.0f, 0.0f),
			atlas::math::Vector2(0.5f, 1.0f)};
		for (auto triangle : m_triangles) {
			triangle->set_uvs(fake);
		}
	}

	std::vector<std::shared_ptr<SmoothMeshUVTriangle>>& Mesh::get_repr()
	{
		return m_triangles;
	}

	void Mesh::dump_to_list(std::vector<std::shared_ptr<Object>>& list)
	{
		for (auto tri : m_triangles) {
			list.push_back(tri);
		}
	}
} // namespace poly::object
