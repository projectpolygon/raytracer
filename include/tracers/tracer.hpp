#pragma once

#include "structures/world.hpp"
#include "structures/world.hpp"

class Tracer {
public:
    Tracer() = default;
    Tracer(World* _world)
      : m_world{_world}
    {

    }

    //virtual Colour trace_ray(math::Ray<math::Vector> const& ray) const;
    virtual Colour trace_ray([[maybe_unused]]math::Ray<math::Vector> const& ray, [[maybe_unused]] const unsigned int depth) const
    {
      return Colour(0.0f, 0.0f, 0.0f);
    }
    //virtual Colour trace_ray(math::Ray<math::Vector> const& ray, float& tmin, const unsigned int depth) const;
protected:
    World* m_world;
};