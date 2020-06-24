#ifndef PINHOLE_HPP
#define PINHOLE_HPP

#include "utilities/utilities.hpp"
#include "cameras/camera.hpp"
#include "structures/world.hpp"
#include "structures/scene_slab.hpp"
#include "structures/surface_interaction.hpp"
#include "objects/object.hpp"

namespace poly::camera {

    class PinholeCamera : public Camera {
    public:

    	using Colour = atlas::math::Vector;

        /*
        * Distance from eye to coordinate grid (larger means tighter rays)
        */
        float m_d;

        /*
        * Constructors
        */
        PinholeCamera();
        PinholeCamera(float d);

        /*
        * Out-of-gamut handling
        */
        Colour colour_validate(Colour const& colour) const;

        /*
        * Scene rendering loops
        */
        void multithread_render_scene(poly::structures::World& world, poly::utils::BMP_info& output);
        void render_scene(poly::structures::World& world) const;

    private:
        void render_slab(std::shared_ptr<poly::structures::scene_slab> slab) const;
    };
}

#endif // !PINHOLE_HPP