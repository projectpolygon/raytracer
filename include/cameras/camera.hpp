#pragma once

#include "structures/scene_slab.hpp"

namespace poly::camera {

    class Camera {
    public:
        Camera() 
            : m_eye{0,0,0}, m_lookat{0,0,-1}, m_up{0,1,0}, m_u{}, m_v{}, m_w{}, m_max_threads{ 1 } 
        {
            // Do nothing
        }
        virtual void render_slab(std::shared_ptr<poly::structures::scene_slab> slab) const = 0;
        virtual void render_scene(poly::structures::World& world) const = 0;

        void eye_set(atlas::math::Point const& eye)
        {
          m_eye = eye;
        }

        void lookat_set(atlas::math::Point const& lookat)
        {
          m_lookat = lookat;
        }

        void upvec_set(atlas::math::Vector const& up)
        {
          m_up = up;
        }

        virtual void uvw_compute()
        {
          atlas::math::Vector w = glm::normalize(m_eye - m_lookat);
          atlas::math::Vector u = glm::normalize(glm::cross(m_up, w));
          atlas::math::Vector v = glm::normalize(glm::cross(w, u));

          this->m_u = u;
          this->m_v = v;
          this->m_w = w;
        };

        void set_max_threads(std::size_t max)
        {
            m_max_threads = max;
        }

    protected:
        atlas::math::Point m_eye;
        atlas::math::Point m_lookat;
        atlas::math::Point m_up;
        atlas::math::Vector m_u, m_v, m_w;
        std::size_t m_max_threads;
    };

}