#pragma once

#include <materials/material.hpp>
#include <structures/bounds.hpp>
#include <memory>

class Object {
public:
    std::shared_ptr<Material> m_material;

    Object()
    {
      m_epsilon = 0.001f;
    }
    virtual bool hit(math::Ray<math::Vector>const& R, ShadeRec& sr) const = 0;
    virtual bool shadow_hit(math::Ray<math::Vector>const& R, float& t) const = 0;

    virtual Bounds3D boundbox_get() const
    {
      return bounds;
    }

    void material_set(std::shared_ptr<Material> const& material)
    {
      m_material = material;
    }
    std::shared_ptr<Material> material_get() const
    {
      return m_material;
    }

protected:
    float m_epsilon;
    Bounds3D bounds;
};