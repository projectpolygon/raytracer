#include <iostream>
#include <cfloat>
#include <zeus/timer.hpp>

#include "paths.hpp"

#include <memory>
#include <structures/shade_rec.hpp>
#include <structures/world.hpp>
#include <materials/material.hpp>
#include <structures/view_plane.hpp>
#include <tracers/whitted_tracer.hpp>
#include <materials/SV_matte.hpp>
#include <samplers/jittered.hpp>
#include <objects/triangle.hpp>
#include <objects/sphere.hpp>
#include <materials/reflective.hpp>
#include <materials/transparent.hpp>
#include <objects/mesh.hpp>
#include <objects/torus.hpp>
#include <lights/ambient_occlusion.hpp>
#include <lights/ambient.hpp>
#include <lights/point_light.hpp>
#include <lights/directional.hpp>
#include <cameras/pinhole.hpp>
#include <structures/KDTree.hpp>

int MESH_MAX_DEPTH = 25;
int MESH_MAX_LEAF_SIZE = 10;

int main()
{
    // Time seed RNG
    srand((unsigned int)time(0));

    std::shared_ptr<poly::structures::ViewPlane> vp = std::make_shared<poly::structures::ViewPlane>();
    vp->hres = 1000;
    vp->vres = 1000;
    vp->max_depth = 5;
    
    // WORLD
    poly::structures::World w = poly::structures::World();
    w.m_vp = vp;
    w.m_background = Colour(0.0f, 0.0f, 0.0f);
    w.m_sampler = std::make_shared<poly::sampler::AA_Jittered>(9, 1);
    w.m_slab_size = 50;
    std::shared_ptr<poly::structures::WhittedTracer> w_tracer = std::make_shared<poly::structures::WhittedTracer>(&w);
    w.m_tracer = w_tracer;
    
    // Textured triangle
    std::vector<math::Point> points3 = { math::Point(-400.0f, -200.0f, 0.0f),
                       math::Point(400.0f, -200.0f, 0.0f),
                       math::Point(-400.0f,200.0f, 0.0f) };
    std::vector<math::Vector2> uvs3 = { math::Vector2(0.0f, 0.0f),
                       math::Vector2(1.0f,0.0f),
                       math::Vector2(0.0, 1.0f) };
    std::vector<math::Vector> flag_normals = { math::Vector(0.0f, 0.0f, 1.0f),
        math::Vector(0.0f, 0.0f, 1.0f) ,
        math::Vector(0.0f, 0.0f, 1.0f) };
    std::shared_ptr<poly::material::SV_Matte> flag_tex
      = std::make_shared<poly::material::SV_Matte>(1.0f, ShaderPath + std::string("flag.png"));

    std::shared_ptr<poly::object::SmoothMeshUVTriangle> flag = std::make_shared<poly::object::SmoothMeshUVTriangle>(
        points3,
        uvs3,
        flag_normals,
        math::Vector(0.0f, -90.0f, -200.0f));
    flag->material_set(flag_tex);

    // Textured triangle
    std::vector<math::Point> points4 = { math::Point(400.0f, -200.0f, 0.0f),
        math::Point(400.0f,200.0f, 0.0f),
        math::Point(-400.0f,200.0f, 0.0f) };
    std::vector<math::Vector2> uvs4 = { math::Vector2(1.0f, 0.0f),
        math::Vector2(1.0f,1.0f),
        math::Vector2(0.0, 1.0f) };
    std::shared_ptr<poly::object::SmoothMeshUVTriangle> flag2 = std::make_shared<poly::object::SmoothMeshUVTriangle>(
        points4,
        uvs4,
        flag_normals,
        math::Vector(0.0f, -90.0f, -200.0f));
    flag2->material_set(flag_tex);

    // Mirrored Sphere
    std::shared_ptr<poly::object::Sphere> mirrorsphere = std::make_shared<poly::object::Sphere>(
        //math::Vector(0.0f, 100.0f, -100.0), 50.0f);
        math::Vector(-90.0f, -50.0f, 100.0f), 30.0f);
    mirrorsphere->material_set(std::make_shared<poly::material::Reflective>(0.9f, 0.05f, 0.5f, Colour(1.0f, 1.0f, 1.0f), 100.0f));
    
    // Transparent sphere
    std::shared_ptr<poly::object::Sphere> bgsphere = std::make_shared<poly::object::Sphere>(
        math::Vector(90.0f, -50.0f, 100.0f), 30.0f);
    bgsphere->material_set(std::make_shared<poly::material::Transparent>(0.28f, 0.75f, 0.04f, 0.7f, Colour(0.1f, 0.1f, 1.0f), 1.02f, 100.0f));

    // Textured SUZANNE MESH
//    Mesh m3 = Mesh(ShaderPath + std::string("suzanne.obj"),
//        ShaderPath + std::string(""),
//        math::Vector(0.0f, 0.0f, 0.0f));
//    m3.material_set(std::make_shared<SV_Matte>(1.0f, ShaderPath + std::string("flag.png")));
//    m3.scale(math::Vector(50.0f, 50.0f, 50.0f));
//    m3.translate(math::Vector(0.0f, 120.0f, 100.0f));
//    m3.fake_uvs(); // Naively set the UV's to cover the image (normally wouldn't do this for a texture, just shows we can)

    // Textured DRAGON MESH
    poly::object::Mesh m3 = poly::object::Mesh(ShaderPath + std::string("dragon.obj"),
                   ShaderPath + std::string(""),
                   math::Vector(0.0f, 0.0f, 0.0f));
    m3.material_set(std::make_shared<poly::material::SV_Matte>(1.0f, ShaderPath + std::string("flag.png")));
    m3.scale(math::Vector(50.0f, 50.0f, 50.0f));
    m3.translate(math::Vector(0.0f, 120.0f, 100.0f));
    m3.fake_uvs(); // Naively set the UV's to cover the image (normally wouldn't do this for a texture, just shows we can)

    // Matte Torus
    std::shared_ptr<poly::object::Torus> torus = std::make_shared<poly::object::Torus>(math::Vector(0.0f, -60.0f, 200.0f), 20.0f, 10.0f);
    torus->material_set(std::make_shared<poly::material::Matte>(0.8f, Colour(1.0f, 0.0f, 0.85f)));

    // AMBIENT LIGHTING
    constexpr bool using_occlusion = true;
    if (using_occlusion) {
        // LIGHT: AMBIENT W/ OCCLUSION
        std::shared_ptr<poly::light::AmbientOcclusion> ambocc = std::make_shared<poly::light::AmbientOcclusion>();
        ambocc->sampler_set(std::make_shared<poly::sampler::AA_Jittered>(25, 1), 2.0f);
        ambocc->colour_set(Colour(1.0f, 1.0f, 1.0f));
        ambocc->radiance_scale(0.3f);
        ambocc->min_amount_set(0.0f);
        w.m_ambient = ambocc;
    }
    else {
        // LIGHT: AMBIENT
        std::shared_ptr<poly::light::AmbientLight> amb = std::make_shared<poly::light::AmbientLight>();
        amb->colour_set(Colour(1.0f, 1.0f, 1.0f));
        amb->radiance_scale(0.3f);
        w.m_ambient = amb;
    }

    // LIGHT: POINT
    std::shared_ptr<poly::light::PointLight> ptlt = std::make_shared<poly::light::PointLight>(
    	math::Vector(0.0f, 100.0f, 500.0f));
    ptlt->radiance_scale(0.8f);
    w.m_lights.push_back(ptlt);

    // LIGHT: DIRECTIONAL
    std::shared_ptr<poly::light::DirectionalLight> dlt = std::make_shared<poly::light::DirectionalLight>();
    dlt->radiance_scale(0.3f);
    dlt->direction_set(math::Vector(1.0f, 1.0f, 1.0f)); 
    //w.m_lights.push_back(dlt);

    // CAMERA
    poly::camera::PinholeCamera cam = poly::camera::PinholeCamera(400.0f);
    cam.eye_set(atlas::math::Point(0.0f, 60.0f, 300.0f));
    cam.lookat_set(atlas::math::Point(0.0f, 0.0f, 0.0f));
    cam.upvec_set(atlas::math::Vector(0.0f, 1.0f, 0.0f));
    cam.uvw_compute();

    // start the timer
    zeus::Timer<float> my_timer = zeus::Timer<float>();
    my_timer.start();

    std::vector<std::shared_ptr<poly::object::Object>> scene;

    // add objects to the scene
    scene.push_back(flag);
    scene.push_back(flag2);
    scene.push_back(mirrorsphere);
    scene.push_back(bgsphere);
    scene.push_back(torus);

    constexpr bool accelerator_enabled = true;
    if (accelerator_enabled) {
        std::vector<std::shared_ptr<poly::object::Object>> object3_data;
        m3.dump_to_list(object3_data);
        scene.push_back(std::make_shared<poly::structures::KDTree>(object3_data, 80, 1, 0.5f, MESH_MAX_LEAF_SIZE, MESH_MAX_DEPTH));

        // Create a KDT on the worlds' scene
        std::shared_ptr<poly::structures::KDTree> kdt
          = std::make_shared<poly::structures::KDTree>(scene, 80, 1, 0.5f, 5, 15);
        w.m_scene.push_back(kdt);
    }
    else {
        m3.dump_to_list(scene);
        w.m_scene = scene;
    }

    constexpr bool mulithreading_enabled = true;
    if (mulithreading_enabled)
    {
        // Render the scene using multiple threads
        cam.multithread_render_scene(w, 12);
    }
    else {
        // Render the scene using a single thread
        cam.render_scene(w);
    }

    std::cout << "\nTime to render was: " << my_timer.elapsed() << std::endl;

    saveToBMP("render.bmp", w.m_vp->hres, w.m_vp->vres, w.m_image);

    return 0;
}

/**
 * Saves a BMP image file based on the given array of pixels. All pixel values
 * have to be in the range [0, 1].
 *
 * @param filename The name of the file to save to.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param image The array of pixels representing the image.
 */
void saveToBMP(std::string const& filename,
	       std::size_t width,
	       std::size_t height,
	       std::vector<Colour> const& image)
{
    std::vector<unsigned char> data(image.size() * 3);

    for (std::size_t i{0}, k{0}; i < image.size(); ++i, k += 3)
    {
	Colour pixel = image[i];
	data[k + 0]  = static_cast<unsigned char>(pixel.r * 255);
	data[k + 1]  = static_cast<unsigned char>(pixel.g * 255);
	data[k + 2]  = static_cast<unsigned char>(pixel.b * 255);
    }

    stbi_write_bmp(filename.c_str(),
		   static_cast<int>(width),
		   static_cast<int>(height),
		   3,
		   data.data());
}
