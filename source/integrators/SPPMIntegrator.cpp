#include <vector>
#include <iostream>
#include <thread>
#include "utilities/utilities.hpp"
#include "integrators/SPPMIntegrator.hpp"

void absorb_photon(std::shared_ptr<poly::material::Material> current_material,
	poly::structures::Photon& photon,
	poly::structures::KDTree& vp_tree,
	std::size_t max_depth,
	poly::structures::World const& world);

void transmit_photon(std::shared_ptr<poly::material::Material> current_material,
	poly::structures::Photon& photon,
	poly::structures::KDTree& vp_tree,
	std::size_t max_depth,
	poly::structures::World const& world,
	float colour_change);

void bounce_photon(std::shared_ptr<poly::material::Material> current_material,
	poly::structures::Photon& photon,
	poly::structures::KDTree& vp_tree,
	std::size_t max_depth,
	poly::structures::World const& world,
	float object_colour_intensity);

enum {
	ABSORB,
	REFLECT,
	TRANSMIT,
	NUM_INTERACTION_TYPES
};

std::size_t TOTAL_NUM = 0;
std::size_t TOTAL_SLAB = 0;

namespace poly::integrators {
	SPPMIntegrator::SPPMIntegrator(std::size_t num_iterations)
		:m_number_iterations{num_iterations}
	{
	
	}

	void SPPMIntegrator::render(poly::structures::World const& world, poly::camera::PinholeCamera const& camera, poly::utils::BMP_info& output)
	{
		// First, create our list of slabs to render with
		output.m_image.clear();
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

		//// Create a square of working space equivalent to the size of our image
		//std::vector<std::vector<Colour>> working_space = std::vector<std::vector<Colour>>(
		//	static_cast<std::size_t>(output.m_end_height) - output.m_start_height,
		//	std::vector<Colour>(static_cast<std::size_t>(output.m_end_width) - output.m_start_width));
		std::size_t current_completion_state = 0;
		std::size_t end_complete_state = slabs.size() * m_number_iterations;
		for (auto slab : slabs) {

			// Repeat the illumination pass for num_iterations
			for (std::size_t iteration{}; iteration < m_number_iterations; ++iteration) {

				/* -------- FIRST PASS -------- */
				/* ------ VISIBLE POINTS ------ */
				std::vector<std::shared_ptr<poly::object::Object>> visible_points = create_visible_points(slab, camera, world_ptr);

				/* -------- SECOND PASS -------- */
				/* ------- PHOTON POINTS ------- */
				photon_mapping(world, visible_points);

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
				/*for (int i = slab->start_y; i < slab->end_y; i++)
				{
					const std::lock_guard<std::mutex> lock(*storage_mutex);
					for (int j = slab->start_x; j < slab->end_x; j++)
					{

						//// 0,0 is in the center of the screen
						int row = (int)i + (int)(world.m_vp->vres / 2);
						int col = (int)j + (int)(world.m_vp->hres / 2);

						// Align the temp slab with the overall scene
						int row_0_indexed = (int)i - slab->start_y;
						int col_0_indexed = (int)j - slab->start_x;

						// Copy out info to the final storage location
						std::size_t storage_height = storage->size();
						std::size_t storage_width= storage->at(0).size();

						std::size_t slab_height = slab->storage->size();
						std::size_t slab_width = slab->storage->at(0).size();
						(void)storage_height;
						(void)storage_width;
						(void)slab_height;
						(void)slab_width;

						storage->at(world.m_vp->vres - row - 1).at(col) = slab->storage->at(row_0_indexed).at(col_0_indexed);
					}
				}*/
				std::cout << "\r                                         ";
				std::cout << "\rLOADING: " << ((float)current_completion_state * 100.0f / end_complete_state) << "% complete. ";
				std::cout << std::flush;
				current_completion_state++;
			}
		}

		// reformat the 2D vector into a single dimensional array
		for (auto row : *(storage))
		{
			for (auto el : row)
			{
				output.m_image.push_back(el);
			}
		}
	}

	std::vector<std::shared_ptr<poly::object::Object>> SPPMIntegrator::create_visible_points(std::shared_ptr<poly::structures::scene_slab> slab,
		poly::camera::PinholeCamera const& camera,
		std::shared_ptr<poly::structures::World> world)
	{
		int total_number_of_pixels = (slab->end_x - slab->start_x) * (slab->end_y - slab->start_y); //slab->storage->size() * slab->storage->at(0).size();
		
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

				int row_0_indexed = (int)i + (slab->world->m_vp->vres)/2;//slab->start_y;
				int col_0_indexed = (int)j + (slab->world->m_vp->hres)/2; //slab->start_x;

				Colour average_factor = Colour(1.0f, 1.0f, 1.0f) * (1.0f / m_number_iterations);
				
				// If we have hit an object, create a visible point at the surface interaction point
				if (hit && sr.m_material) {
					
					// Shade the point directly
					//slab->storage->at(slab->world->m_vp->vres - row_0_indexed - 1).at(col_0_indexed) += (sr.m_material->shade(sr, *(slab->world))) * average_factor;

					// Add this visible point to our vector
					visiblePoints.push_back(std::make_shared<poly::integrators::VisiblePoint>(j, i, sr.hitpoint_get(), -ray.d, Colour(1.0,1.0,1.0), sr.m_material, slab));
				}
				else {
					slab->storage->at(slab->world->m_vp->vres - row_0_indexed - 1).at(col_0_indexed) += world->m_background * average_factor;;
				}

			}
		}
		return visiblePoints;
	}

	void SPPMIntegrator::photon_mapping(const poly::structures::World& world, std::vector<std::shared_ptr<poly::object::Object>>& vp_list)
	{
		poly::structures::KDTree vp_tree(vp_list, 80, 30, 0.75f, 10, 50);

		// TODO: Make configurable
		std::size_t photon_count = 100000;

		for (auto light : world.m_lights) {
			for (std::size_t i{ 0 }; i < photon_count; ++i) {
				float x, y, z;
				do {
					x = 2.0f * ((((float)(rand() % 1000)) / 1000.0f)) - 1.0f;
					y = 2.0f * ((((float)(rand() % 1000)) / 1000.0f)) - 1.0f;
					z = 2.0f * ((((float)(rand() % 1000)) / 1000.0f)) - 1.0f;
				} while (x * x + y * y + z * z > 1.0f);

				math::Vector d{ x, y, z };
				math::Point o{ light->location() };
				math::Ray<math::Vector> photon_ray{ o, d };
				structures::SurfaceInteraction si;

				bool is_hit{ false };
				for (auto obj : world.m_scene) {
					if (obj->hit(photon_ray, si))
						is_hit = true;
				}

				if (is_hit) {
					poly::structures::Photon photon = poly::structures::Photon(photon_ray, si.hitpoint_get(), si.m_normal, light->ls() / photon_count, 0);
					//si.m_material->absorb_photon(photon, vp_tree, world.m_vp->max_depth, world);
					absorb_photon(si.m_material, photon, vp_tree, (std::size_t)world.m_vp->max_depth, world);
				}

			}
		}
	}

	/*
	===============================
	-------- VISIBLE POINT --------
	===============================
	*/

	VisiblePoint::VisiblePoint(
		int x_, 
		int y_, 
		math::Point const& point_, 
		math::Vector const& incoming_ray_, 
		Colour amount_, 
		std::shared_ptr<poly::material::Material> material_,
		std::shared_ptr<poly::structures::scene_slab> slab)
		: index_x{ x_ }, index_y{ y_ }, point(point_), w_i(incoming_ray_), amount{ amount_ }, material(material_), m_slab{slab}
	{
		// ensure that our bounds are set
		bounds = poly::structures::Bounds3D(point, point);
	}
	bool VisiblePoint::hit([[maybe_unused]]math::Ray<math::Vector> const& R, [[maybe_unused]] poly::structures::SurfaceInteraction& sr) const
	{
		return true;
	}
	bool VisiblePoint::shadow_hit([[maybe_unused]] math::Ray<math::Vector> const& R, [[maybe_unused]] float& t) const
	{
		return false;
	}
	void VisiblePoint::add_contribution(poly::structures::Photon const& photon)
	{
		//std::cout << "WOWEE" << std::endl;
		int row_0_indexed = (int)index_y + (m_slab->world->m_vp->vres) / 2;//slab->start_y;
		int col_0_indexed = (int)index_x + (m_slab->world->m_vp->hres) / 2; //slab->start_x;
		float intensity = photon.intensity();
		(void)(intensity);

		m_slab->storage->at(m_slab->world->m_vp->vres - row_0_indexed - 1).at(col_0_indexed) += Colour(1.0f, 1.0f, 1.0f) * 1.0f;//intensity;
		//(void)photon;
	}


}
/**
Steps:
the entire algorithm repeats for N iterations

1. Shoot out rays from camera. At every intersection with an object (1 ONLY), create a visiblePoint. Store these "Visible Points" in a KD-Tree, indexed based on location in the scene.

2. Shoot out rays from each light, intersection against the scenery. On each interstedtion, check nearby visible point KD-Tree. For each nearby visible point, add the photon's value to it's light contribution 

PSEUDOCODE FOR ALGORITHM

*/
void absorb_photon(std::shared_ptr<poly::material::Material> current_material,
	poly::structures::Photon& photon,
	poly::structures::KDTree& vp_tree,
	std::size_t max_depth,
	poly::structures::World const& world)
{
	// If the max depth for recursion is reached, stop here
	if (photon.depth() >= max_depth) {
		//photons.push_back(photon);
		std::vector<std::shared_ptr<poly::object::Object>> nearby_VPs = vp_tree.get_nearest_to_point(photon.point(), 1.0f, 10);
		for (auto vp : nearby_VPs) {
			vp->add_contribution(photon);
		}
		// Add contribution to nearby VP's
		return;
	}

	if (current_material->m_type == ABSORB) {
		// Assess whether or not this should be bounced by taking the intensity of the diffuse component of the material
		float partition = current_material->get_diffuse_strength();
		float rgn = (float(rand()) / float(std::numeric_limits<int>::max())); // TODO: fix this, it is not portable
		if (rgn > partition) {
			// Bounce the photon off this material
			bounce_photon(current_material, photon, vp_tree, max_depth, world, partition);
		}
		// TODO: Add contribution to nearby VP's if no bounce!!!
		std::vector<std::shared_ptr<poly::object::Object>> nearby_VPs = vp_tree.get_nearest_to_point(photon.point(), 10.0f, 5);
		for (auto vp : nearby_VPs) {
			vp->add_contribution(photon);
		}
		return;
	}
	else if (current_material->m_type == REFLECT) {
		float specular_kd = current_material->get_specular_strength();
		float reflective_kd = current_material->get_reflective_strength();
		float diffuse_kd = current_material->get_diffuse_strength();
		float total = reflective_kd + diffuse_kd + specular_kd;

		float rgn = (float(rand()) / float(std::numeric_limits<int>::max())) * total;

		if (rgn < reflective_kd) {
			bounce_photon(current_material, photon, vp_tree, max_depth, world, (photon.intensity() * reflective_kd / total));
		}
		photon.intensity(photon.intensity() * (1 - (reflective_kd / total)));
		//photons.push_back(photon);
	}
	else if (current_material->m_type == TRANSMIT) {

		float transparent_kt = current_material->get_refractive_strength();
		float specular_kd = current_material->get_specular_strength();
		float reflective_kd = current_material->get_reflective_strength();
		float diffuse_kd = current_material->get_diffuse_strength();
		float total = transparent_kt + specular_kd + reflective_kd + diffuse_kd;

		// Random number in the range 0 to total
		float random_number = (float(rand()) / float(std::numeric_limits<int>::max())) * total;

		if (random_number < transparent_kt) {
			transmit_photon(current_material, photon, vp_tree, max_depth, world, photon.intensity() * transparent_kt / total);
		}
		else if (random_number >= transparent_kt && random_number < transparent_kt + reflective_kd) {
			bounce_photon(current_material, photon, vp_tree, max_depth, world, (reflective_kd + reflective_kd) / total * photon.intensity());
		}
		photon.intensity(photon.intensity() * diffuse_kd / total);
		// Add photon contribution to VP
	}
}

void bounce_photon(std::shared_ptr<poly::material::Material> current_material,
	poly::structures::Photon& photon,
	poly::structures::KDTree& vp_tree,
	std::size_t max_depth,
	poly::structures::World const& world,
	float object_colour_intensity)
{
	poly::structures::SurfaceInteraction si;
	// Get the new ray direction from the photon
	atlas::math::Ray<atlas::math::Vector> photon_ray = photon.reflect_ray();

	// Hit new objects with this ray
	bool is_hit{ false };
	for (auto obj : world.m_scene) {
		if (obj->hit(photon_ray, si))
		{
			is_hit = true;
		}
	}

	// If we hit an object, get its material and propogate this photon
	if (is_hit) {
		poly::structures::Photon reflected_photon = poly::structures::Photon(photon_ray,
			si.hitpoint_get(),
			si.m_normal,
			photon.intensity() * (1 - object_colour_intensity),
			photon.depth() + 1);
		absorb_photon(si.m_material, reflected_photon, vp_tree, max_depth, world);
	}
	float new_intensity = photon.intensity() * object_colour_intensity;
	photon.intensity(new_intensity);
}

void transmit_photon(std::shared_ptr<poly::material::Material> current_material, 
	poly::structures::Photon& photon, 
	poly::structures::KDTree& vp_tree, 
	std::size_t max_depth, 
	poly::structures::World const& world, 
	float colour_change)
{
	poly::structures::SurfaceInteraction si;
	si.m_normal = photon.normal();
	atlas::math::Vector wi = photon.wi().d;
	atlas::math::Vector wt;

	current_material->sample_f(si, wi, wt);
	//m_transmitted_btdf->sample_f(si, wi, wt);

	atlas::math::Ray<atlas::math::Vector> photon_ray{ photon.point(), wt }; // This was using wi instead of wt before the switch to functions in this class

	// Send the transmitted ray through the scene
	bool is_hit{ false };
	for (auto obj : world.m_scene) {
		if (obj->hit(photon_ray, si))
			is_hit = true;
	}

	// If we hit an object, possibly generate new rays, otherwise, simply change the photons intensity 
	if (is_hit) {
		poly::structures::Photon reflected_photon = poly::structures::Photon(photon_ray,
			si.hitpoint_get(),
			si.m_normal,
			photon.intensity() * (1 - colour_change),
			photon.depth() + 1);
		absorb_photon(si.m_material, reflected_photon, vp_tree, max_depth, world);
	}
	float new_intensity = photon.intensity() * colour_change;
	photon.intensity(new_intensity);
}
