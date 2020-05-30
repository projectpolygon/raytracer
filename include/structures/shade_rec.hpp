#pragma once

#ifndef SHADEREC_HPP
#define SHADEREC_HPP

#include <memory>
#include "structures/world.hpp"
#include "utilities/utilities.hpp"
#include "materials/material.hpp"

namespace poly::material {class Material;}

namespace poly::structures {

    //class World;

    class ShadeRec {
    public:
        float m_tmin;
        float u, v;
        unsigned int depth;
        atlas::math::Ray<atlas::math::Vector> m_ray;
        World& m_world;
        std::shared_ptr<poly::material::Material> m_material;
        Colour m_colour;
        atlas::math::Normal m_normal;

        ShadeRec(World& world);
        atlas::math::Vector hitpoint_get() const;
    };
}

#endif // !SHADEREC_HPP