#include <vector>
#include <iostream>
#include <thread>
#include "utilities/utilities.hpp"
#include "integrators/SPPMIntegrator.hpp"


std::size_t TOTAL_NUM = 0;
std::size_t TOTAL_SLAB = 0;

namespace poly::integrators {
	SPPMIntegrator::SPPMIntegrator()
	{
	
	}

	void SPPMIntegrator::render(poly::structures::World const& world, poly::camera::PinholeCamera const& camera, poly::utils::BMP_info& output)
	{
		(void)output;

		// First, create our list of slabs to render with
		std::shared_ptr<std::vector<std::vector<Colour>>> storage = std::make_shared<std::vector<std::vector<Colour>>>(world.m_vp->vres, std::vector<Colour>(world.m_vp->hres));
		std::shared_ptr<std::mutex> storage_mutex = std::make_shared<std::mutex>();
		std::vector<std::thread> thread_list;
		std::shared_ptr<poly::structures::World> world_ptr = std::make_shared<poly::structures::World>(world);

		std::vector<std::shared_ptr<poly::structures::scene_slab>> slabs = poly::structures::generate_slabs(
			world.m_start_width, world.m_end_width,
			world.m_start_height, world.m_end_height,
			world.m_slab_size,
			world_ptr,
			storage_mutex,
			storage);

		for (auto slab : slabs) {

			// Repeat the illumination pass for num_iterations
			std::size_t num_iterations = 1;
			for (std::size_t iteration{}; iteration < num_iterations; ++iteration) {

				/* -------- FIRST PASS -------- */
				/* ------ VISIBLE POINTS ------ */
				std::vector<std::shared_ptr<poly::object::Object>> visible_points = create_visible_points(slab, camera, world_ptr);
				poly::structures::KDTree visible_point_tree(visible_points, 80, 30, 0.75f, 10, 50);

				/* -------- SECOND PASS -------- */
				/* ------- PHOTON POINTS ------- */
				/*
				For each light
					shoot photons from the light
					for each photon shot
						intersect against the scene
						if hit
							gather N nearby visible points
							add photon to each of the N points (update using pointer to location on film inside the VisiblePoint
						calculate next photon bounce, or terminate photon
				*/

				// Render image

			}

		}

	}

	std::vector<poly::structures::Photon> photon_mapping(const poly::structures::World& world)
	{
		std::size_t photon_count = 100000;

		std::vector<poly::structures::Photon> photons;

		for (auto light: world.m_lights) {
			for (std::size_t i{0}; i < photon_count; ++i) {
				float x, y, z;
				do {
					x = 2.0f * (float(rand()) / float(std::numeric_limits<int>::max())) - 1.0f;
					y = 2.0f * (float(rand()) / float(std::numeric_limits<int>::max())) - 1.0f;
					z = 2.0f * (float(rand()) / float(std::numeric_limits<int>::max())) - 1.0f;
				} while (x * x + y * y + z * z);

				math::Vector d{x, y, z};
				math::Point o{light->location()};
				math::Ray<math::Vector> photon_ray{o, d};
				structures::SurfaceInteraction si;

				bool is_hit{false};
				for (auto obj: world.m_scene) {
					if (obj->hit(photon_ray, si))
						is_hit = true;
				}

				if (is_hit) {
					poly::structures::Photon photon = poly::structures::Photon(photon_ray,
						si.hitpoint_get(), si.m_normal, light->ls() / photon_count, 0);
					si.m_material->trace_photon(photon, photons, world.m_vp->max_depth, world.m_scene);
				}

			}
		}
		return photons;
	}

	std::vector<std::shared_ptr<poly::object::Object>> SPPMIntegrator::create_visible_points(std::shared_ptr<poly::structures::scene_slab> slab,
		poly::camera::PinholeCamera const& camera,
		std::shared_ptr<poly::structures::World> world)
	{
		std::size_t total_number_of_pixels = slab->storage->size() * slab->storage->size();
		
		// Create an array of visible points (so that it can be placed in the KD tree!)
		std::vector<std::shared_ptr<poly::object::Object>> visiblePoints;
		visiblePoints.reserve(total_number_of_pixels);

		for (int i = slab->start_y; i < slab->end_y; i++){
			for (int j = slab->start_x; j < slab->end_x; j++){
				
				// Shoot a ray into the scene, closest intersection will become a "visible point"
				poly::structures::SurfaceInteraction sr;
				sr.m_colour = world->m_background;
				sr.depth = 0;
				atlas::math::Ray<atlas::math::Vector> ray = camera.get_ray(i, j, *world);

				// Iterate over scene, tracking hitpoints
				bool hit = false;
				for (std::shared_ptr<poly::object::Object> obj : world->m_scene)
				{
					if (obj->hit(ray, sr))
					{
						hit = true;
					}
				}
				
				// If we have hit an object, create a visible point at the surface interaction point
				if (hit && sr.m_material) {
					// Add this visible point to our vector
					visiblePoints.push_back(std::make_shared<poly::integrators::VisiblePoint>(j, i, sr.hitpoint_get(), -ray.d, Colour{1.0,1.0,1.0}, sr.m_material));
				}

			}
		}
		return visiblePoints;
	}

	/*
	===============================
	-------- VISIBLE POINT --------
	===============================
	*/

	VisiblePoint::VisiblePoint(int x_, int y_, math::Point const& point_, math::Vector const& incoming_ray_, Colour amount_, std::shared_ptr<poly::material::Material> material_)
		: index_x{ x_ }, index_y{ y_ }, point(point_), w_i(incoming_ray_), amount{amount_}, material(material_)
	{
		// ensure that our bounds are set
		bounds = poly::structures::Bounds3D(point, point);
	}
	bool VisiblePoint::hit([[maybe_unused]]math::Ray<math::Vector> const& R, [[maybe_unused]] poly::structures::SurfaceInteraction& sr) const
	{
		return false;
	}
	bool VisiblePoint::shadow_hit([[maybe_unused]] math::Ray<math::Vector> const& R, [[maybe_unused]] float& t) const
	{
		return false;
	}
}
/**
Steps:
the entire algorithm repeats for N iterations

1. Shoot out rays from camera. At every intersection with an object (1 ONLY), create a visiblePoint. Store these "Visible Points" in a KD-Tree, indexed based on location in the scene.

2. Shoot out rays from each light, intersection against the scenery. On each interstedtion, check nearby visible point KD-Tree. For each nearby visible point, add the photon's value to it's light contribution 

PSEUDOCODE FOR ALGORITHM

*/
