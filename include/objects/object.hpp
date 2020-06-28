#ifndef OBJECT_HPP
#define OBJECT_HPP

namespace poly::object { class Object; }
#include "structures/KDTree.hpp"
#include "materials/material.hpp"
//#include "structures/surface_interaction.hpp"
#include "structures/bounds.hpp"

namespace poly::material { class Material; }
namespace poly::structures { class SurfaceInteraction; }

namespace poly::object {

	class Object {
	public:
		std::shared_ptr<poly::material::Material> m_material;

		Object()
		{
			m_epsilon = 0.001f;
		}
		virtual bool hit(atlas::math::Ray<math::Vector>const& R,
			poly::structures::SurfaceInteraction& sr) const = 0;
		virtual bool shadow_hit(atlas::math::Ray<atlas::math::Vector>const& R, float& t) const = 0;

		virtual poly::structures::Bounds3D boundbox_get() const
		{
			return bounds;
		}

		void material_set(std::shared_ptr<poly::material::Material> const& material)
		{
			m_material = material;
		}
		std::shared_ptr<poly::material::Material> material_get() const
		{
			return m_material;
		}

	protected:
		float m_epsilon;
		poly::structures::Bounds3D bounds;
	};
}

#endif // !OBJECT_HPP