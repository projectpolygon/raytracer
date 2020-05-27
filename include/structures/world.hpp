#pragma once

class Light;

#include <samplers/sampler.hpp>
#include "view_plane.hpp"
#include "tracers/tracer.hpp"
#include <objects/object.hpp>


class World {
public:
    Colour m_background{}; // Default pixel colour

    std::shared_ptr<Sampler> m_sampler; // The sampler for AA

    std::vector<std::shared_ptr<Object>> m_scene; // Objects in our scene

    std::shared_ptr<Light> m_ambient; // Ambient light in our scene (gets handled specially)
    std::vector<std::shared_ptr<Light>> m_lights; // Lights in our scene
    std::vector<Colour> m_image; // Output as 1D array

    std::shared_ptr<ViewPlane> m_vp;
    //std::shared_ptr<AcceleratorStruct> m_acceleratorStructure;
    std::shared_ptr<Tracer> m_tracer;

    unsigned int m_slab_size{};

    World() = default;/*:m_acceleratorStructure{ nullptr } */
    ~World() = default;

    //void accelerator_set(std::shared_ptr<AcceleratorStruct> acc_struct)
    //{
    //	m_acceleratorStructure = acc_struct;
    //}
};