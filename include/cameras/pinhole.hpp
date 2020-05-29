#pragma once

#include "cameras/camera.hpp"
#include "utilities/utilities.hpp"
#include "structures/world.hpp"
#include "structures/scene_slab.hpp"

namespace poly::camera {

    class PinholeCamera : public Camera {
    public:
        float m_d;

        PinholeCamera(float d);

        Colour colour_validate(Colour const& colour) const;

        void multithread_render_scene(poly::structures::World& world, unsigned int num_threads);

        void render_scene(poly::structures::World& world) const;

    protected:
        void render_slab(std::shared_ptr<poly::structures::scene_slab> slab) const;
    };
}