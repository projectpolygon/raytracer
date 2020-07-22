#include "integrators/SPPMIntegrator.hpp"
#include "samplers/sampler.hpp"
#include "structures/world.hpp"
#include "utilities/utilities.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atlas/math/random.hpp>

static constexpr float direct_shading_strength		   = 0.5f;
static constexpr float photon_strength_multiplier	   = 100.0f;
static constexpr std::size_t num_photons_per_iteration = 100'000;

/*
===============================
--------- PROTOTYPES ----------
===============================
*/

void absorb_photon(std::shared_ptr<poly::material::Material> current_material,
				   poly::structures::Photon &photon,
				   poly::structures::KDTree &vp_tree,
				   std::size_t max_depth,
				   poly::structures::World const &world,
				   std::shared_ptr<std::mutex> storage_mutex);

void transmit_photon(std::shared_ptr<poly::material::Material> current_material,
					 poly::structures::Photon &photon,
					 poly::structures::KDTree &vp_tree,
					 std::size_t max_depth,
					 poly::structures::World const &world,
					 float colour_change,
					 std::shared_ptr<std::mutex> storage_mutex);

void bounce_photon(std::shared_ptr<poly::material::Material> current_material,
				   poly::structures::Photon &photon,
				   poly::structures::KDTree &vp_tree,
				   std::size_t max_depth,
				   poly::structures::World const &world,
				   float object_colour_intensity,
				   std::shared_ptr<std::mutex> storage_mutex);

void absorb_vp(poly::structures::SurfaceInteraction &sr,
			   atlas::math::Ray<atlas::math::Vector> const &ray,
			   std::shared_ptr<poly::structures::World> world,
			   Colour &amount);

void transmit_vp(poly::structures::SurfaceInteraction &sr,
				 atlas::math::Ray<atlas::math::Vector> const &ray,
				 std::shared_ptr<poly::structures::World> world,
				 Colour &amount);

void bounce_vp(poly::structures::SurfaceInteraction &sr,
			   atlas::math::Ray<atlas::math::Vector> const &ray,
			   std::shared_ptr<poly::structures::World> world,
			   Colour &amount);

namespace poly::integrators
{
	/*
	===============================
	------- PHOTON MAPPER ---------
	===============================
	*/
	SPPMIntegrator::SPPMIntegrator(std::size_t num_iterations) :
		m_number_iterations{num_iterations}
	{}

	void SPPMIntegrator::render(poly::structures::World const &world,
								poly::camera::PinholeCamera const &camera,
								poly::utils::BMP_info &output)
	{
		// First, create our list of slabs to render with
		output.m_image.clear();
		std::shared_ptr<std::vector<std::vector<Colour>>> storage =
			std::make_shared<std::vector<std::vector<Colour>>>(
				world.m_vp->vres, std::vector<Colour>(world.m_vp->hres));

		std::vector<std::shared_ptr<std::vector<std::vector<Colour>>>>
			temp_storage;

		for (std::size_t iteration{0}; iteration < m_number_iterations;
			 ++iteration) {
			std::shared_ptr<std::vector<std::vector<Colour>>> temp =
				std::make_shared<std::vector<std::vector<Colour>>>(
					world.m_vp->vres, std::vector<Colour>(world.m_vp->hres));
			temp_storage.push_back(temp);
		}

		std::shared_ptr<std::mutex> storage_mutex =
			std::make_shared<std::mutex>();
		std::vector<std::thread> thread_list;
		std::shared_ptr<poly::structures::World> world_ptr =
			std::make_shared<poly::structures::World>(world);

		// Repeat the illumination pass for num_iterations
		for (std::size_t iteration{0}; iteration < m_number_iterations;
			 ++iteration) {
			// Create a new thread for each iteration

			thread_list.emplace_back(std::thread([=]() {
				/* -------- FIRST PASS -------- */
				/* ------ VISIBLE POINTS ------ */
				std::vector<std::shared_ptr<poly::object::Object>>
					visible_points = create_visible_points(
						world.m_start_width - (world.m_vp->hres / 2),
						world.m_start_height - (world.m_vp->vres / 2),
						world.m_end_width - (world.m_vp->hres / 2),
						world.m_end_height - (world.m_vp->vres / 2),
						temp_storage.at(iteration),
						camera,
						world_ptr);

				/* -------- SECOND PASS -------- */
				/* ------- PHOTON POINTS ------- */
				photon_mapping(world, visible_points, storage_mutex);
			}));
		}
		// Joining the threads
		for (std::thread &t : thread_list) {
			t.join();
		}
		for (auto &iter : temp_storage) {
			for (std::size_t i{0};
				 i < static_cast<std::size_t>(world.m_vp->vres);
				 ++i) {
				for (std::size_t j{0};
					 j < static_cast<std::size_t>(world.m_vp->hres);
					 ++j) {
					storage->at(i).at(j) += iter->at(i).at(j);
				}
			}
		}

		float scale_factor = (1 / static_cast<float>(m_number_iterations));

		// reformat the 2D vector into a single dimensional array
		for (auto &row : *(storage)) {
			for (auto element : row) {
				output.m_image.push_back(
					poly::utils::colour_average_max(element * scale_factor));
			}
		}
	}

	std::vector<std::shared_ptr<poly::object::Object>>
	SPPMIntegrator::create_visible_points(
		int start_x,
		int start_y,
		int end_x,
		int end_y,
		std::shared_ptr<std::vector<std::vector<Colour>>> storage,
		poly::camera::PinholeCamera const &camera,
		std::shared_ptr<poly::structures::World> world)
	{
		int total_number_of_pixels = (end_x - start_x) * (end_y - start_y);

		// Create an array of visible points  placed in the KD tree)
		std::vector<std::shared_ptr<poly::object::Object>> visiblePoints;
		visiblePoints.reserve(total_number_of_pixels);

		for (int i = start_y; i < end_y; i++) {
			for (int j = start_x; j < end_x; j++) {
				// Shoot a ray into the scene, closest intersection will become
				// a "visible point"
				poly::structures::SurfaceInteraction sr;
				sr.m_colour = world->m_background;
				sr.depth	= 0;
				atlas::math::Ray<atlas::math::Vector> ray =
					camera.get_ray(i, j, *world);

				// Iterate over scene, tracking hitpoints
				bool hit = false;
				for (std::shared_ptr<poly::object::Object> obj :
					 world->m_scene) {
					if (obj->hit(ray, sr)) {
						hit = true;
					}
				}

				// Find the index in our film where we will link this ray to
				int row_0_indexed =
					static_cast<int>(i) + (world->m_vp->vres) / 2;
				int col_0_indexed =
					static_cast<int>(j) + (world->m_vp->hres) / 2;

				// If we have hit an object, create a visible point at the
				// surface interaction point
				if (hit && sr.m_material) {
					// Shade the point directly
					storage->at(world->m_vp->vres - row_0_indexed - 1)
						.at(col_0_indexed) +=
						(sr.m_material->shade(sr, *(world)));

					Colour amount{1.0f, 1.0f, 1.0f};

					// Recursively bounce the photon around the scene
					absorb_vp(sr, ray, world, amount);

					// Add this visible point to our vector
					visiblePoints.push_back(
						std::make_shared<poly::integrators::VisiblePoint>(
							j,
							i,
							sr.get_hitpoint(),
							-ray.d,
							amount,
							sr.m_material,
							storage,
							world));
				}
			}
		}
		return visiblePoints;
	}

	void SPPMIntegrator::photon_mapping(
		const poly::structures::World &world,
		std::vector<std::shared_ptr<poly::object::Object>> &vp_list,
		std::shared_ptr<std::mutex> storage_mutex)
	{
		poly::structures::KDTree vp_tree(vp_list, 80, 30, 0.75f, 10, -1);

		constexpr std::size_t photon_count =
			num_photons_per_iteration; // TODO: Make configurable by end user

		for (auto &light : world.m_lights) {
			for (std::size_t i{0}; i < photon_count; ++i) {
				float x, y, z;
				do {
					x = 2.0f * (((float)(rand() % 10000)) / 10000.0f) - 1.0f;
					y = 2.0f * (((float)(rand() % 10000)) / 10000.0f) - 1.0f;
					z = 2.0f * (((float)(rand() % 10000)) / 10000.0f) - 1.0f;
				} while (x * x + y * y + z * z > 1.0f);

				math::Vector d{x, y, z};
				// d = glm::normalize(d); // TODO replace with random class
				math::Point o{light->location()};
				math::Ray<math::Vector> photon_ray{o, d};
				structures::SurfaceInteraction si;

				bool is_hit{false};
				for (auto &obj : world.m_scene) {
					if (obj->hit(photon_ray, si)) {
						is_hit = true;
					}
				}

				if (is_hit) {
					poly::structures::Photon photon = poly::structures::Photon(
						photon_ray,
						si.get_hitpoint(),
						si.m_normal,
						m_number_iterations * photon_strength_multiplier *
							light->ls() /
							static_cast<float>(
								photon_count), // scale the photon's strength by
											   // 1000
						0);

					// Using this photon, absorb will determine the behaviour of
					// when to bounce, absorb, or transmit
					absorb_photon(si.m_material,
								  photon,
								  vp_tree,
								  (std::size_t)world.m_vp->max_depth,
								  world,
								  storage_mutex);
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
		math::Point const &point_,
		math::Vector const &incoming_ray_,
		Colour amount_,
		std::shared_ptr<poly::material::Material> material_,
		std::shared_ptr<std::vector<std::vector<Colour>>> storage_,
		std::shared_ptr<poly::structures::World> world_) :
		index_x{x_},
		index_y{y_},
		point(point_),
		w_i(incoming_ray_),
		amount{amount_},
		surface_material{material_},
		m_storage{storage_},
		m_world{world_}
	{
		// Ensure that our point gets its bounds set
		assert(surface_material);
		bounds = poly::structures::Bounds3D(point, point);
	}
	bool VisiblePoint::hit(
		[[maybe_unused]] math::Ray<math::Vector> const &R,
		[[maybe_unused]] poly::structures::SurfaceInteraction &sr) const
	{
		if (glm::dot(R.o - point, R.o - point) < R.d.x * R.d.x) {
			return true;
		}
		else {
			return false;
		}
	}

	bool
	VisiblePoint::shadow_hit([[maybe_unused]] math::Ray<math::Vector> const &R,
							 [[maybe_unused]] float &t) const
	{
		return false;
	}

	void
	VisiblePoint::add_contribution(poly::structures::Photon const &photon,
								   std::shared_ptr<std::mutex> storage_mutex)
	{
		int row_0_indexed = (int)index_y + (m_world->m_vp->vres) / 2;
		int col_0_indexed = (int)index_x + (m_world->m_vp->hres) / 2;

		float dist_x = point.x - photon.point().x;
		float dist_y = point.y - photon.point().y;
		float dist_z = point.z - photon.point().z;

		float dist_to_vp =
			std::max(1.0f, dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);

		Colour intensity = amount * photon.intensity();

		m_storage->at(m_world->m_vp->vres - row_0_indexed - 1)
			.at(col_0_indexed) += intensity / dist_to_vp;
	}

} // namespace poly::integrators

/*
======================================
------ VISIBLE POINT BEHAVIOUR -------
======================================
*/

void absorb_vp(poly::structures::SurfaceInteraction &sr,
			   atlas::math::Ray<atlas::math::Vector> const &ray,
			   std::shared_ptr<poly::structures::World> world,
			   Colour &amount)
{
	// If we've reached the max depth, we stay here!
	if (world->m_vp->max_depth <= sr.depth) {
		return;
	}

	++sr.depth;

	std::shared_ptr<poly::material::Material> current_material = sr.m_material;

	// Scale the amount of light transmitted through to the film by the colour
	// of the transmitted material
	amount *= current_material->get_hue(sr.get_hitpoint());

	if (current_material->m_type == poly::structures::InteractionType::ABSORB) {
		// Assess whether or not this should be bounced by taking the intensity
		// of the diffuse component of the material
		float partition = current_material->get_diffuse_strength();
		float rgn		= (((float)(rand() % 10000)) /
					   10000.0f); // TODO: fix this, it is not portable
		if (rgn > partition) {
			// Bounce the photon off this material
			bounce_vp(sr, ray, world, amount);
		}
		return;
	}
	else if (current_material->m_type ==
			 poly::structures::InteractionType::REFLECT) {
		float specular_kd	= current_material->get_specular_strength();
		float reflective_kd = current_material->get_reflective_strength();
		float diffuse_kd	= current_material->get_diffuse_strength();
		float total			= reflective_kd + diffuse_kd + specular_kd;

		float rgn = (((float)(rand() % 10000)) / 10000.0f) * total;

		if (rgn < reflective_kd) {
			bounce_vp(sr, ray, world, amount);
		}
	}
	else if (current_material->m_type ==
			 poly::structures::InteractionType::TRANSMIT) {
		float transparent_kt = current_material->get_refractive_strength();
		float specular_kd	 = current_material->get_specular_strength();
		float reflective_kd	 = current_material->get_reflective_strength();
		float diffuse_kd	 = current_material->get_diffuse_strength();
		float total = transparent_kt + specular_kd + reflective_kd + diffuse_kd;

		// Random number in the range 0 to total
		float random_number = (((float)(rand() % 10000)) / 10000.0f) * total;

		if (random_number < transparent_kt) {
			transmit_vp(sr, ray, world, amount);
		}
		else if (random_number >= transparent_kt &&
				 random_number < transparent_kt + reflective_kd) {
			bounce_vp(sr, ray, world, amount);
		}
	}
}

void bounce_vp(poly::structures::SurfaceInteraction &sr,
			   atlas::math::Ray<atlas::math::Vector> const &ray,
			   std::shared_ptr<poly::structures::World> world,
			   Colour &amount)
{
	math::Ray<math::Vector> reflected_ray(
		sr.get_hitpoint(),
		poly::utils::reflect_over_normal(-ray.d, sr.m_normal));

	sr.m_tmin = std::numeric_limits<float>::max();

	// Hit new objects with this ray
	bool is_hit{false};
	for (auto &obj : world->m_scene) {
		if (obj->hit(reflected_ray, sr)) {
			is_hit = true;
		}
	}

	// If we hit an object, get its material and propogate this vp
	if (is_hit) {
		absorb_vp(sr, reflected_ray, world, amount);
	}
}

void transmit_vp(poly::structures::SurfaceInteraction &sr,
				 atlas::math::Ray<atlas::math::Vector> const &ray,
				 std::shared_ptr<poly::structures::World> world,
				 Colour &amount)
{
	std::shared_ptr<poly::material::Material> current_material = sr.m_material;
	atlas::math::Vector wi									   = -ray.d;
	atlas::math::Vector wt;

	current_material->sample_f(sr, wi, wt);

	math::Ray<math::Vector> transmitted_ray(sr.get_hitpoint(), wt);

	sr.depth++;
	sr.m_tmin = std::numeric_limits<float>::max();

	// Send the transmitted ray through the scene
	bool is_hit{false};
	for (auto &obj : world->m_scene) {
		if (obj->hit(transmitted_ray, sr)) {
			is_hit = true;
		}
	}

	// If we hit an object, possibly generate new rays, otherwise, simply change
	// the photons intensity
	if (is_hit) {
		absorb_vp(sr, transmitted_ray, world, amount);
	}
}

/*
===============================
------ PHOTON BEHAVIOUR -------
===============================
*/
/**
Steps:
the entire algorithm repeats for N iterations

1. Shoot out rays from camera. At every intersection with an object (1 ONLY),
create a visiblePoint. Store these "Visible Points" in a KD-Tree, indexed based
on location in the scene.

2. Shoot out rays from each light, intersection against the scenery. On each
interstedtion, check nearby visible point KD-Tree. For each nearby visible
point, add the photon's value to it's light contribution

PSEUDOCODE FOR ALGORITHM

*/
void absorb_photon(std::shared_ptr<poly::material::Material> current_material,
				   poly::structures::Photon &photon,
				   poly::structures::KDTree &vp_tree,
				   std::size_t max_depth,
				   poly::structures::World const &world,
				   std::shared_ptr<std::mutex> storage_mutex)
{
	constexpr float max_distance_to_visible_point = 20.0f;
	// If the max depth for recursion is reached, stop here
	if (photon.depth() >= max_depth) {
		// photons.push_back(photon);
		std::vector<std::shared_ptr<poly::object::Object>> nearby_VPs =
			vp_tree.get_nearest_to_point(photon.point(),
										 max_distance_to_visible_point);
		for (auto &vp : nearby_VPs) {
			vp->add_contribution(photon, storage_mutex);
		}
		// Add contribution to nearby VP's
		return;
	}

	if (current_material->m_type == poly::structures::InteractionType::ABSORB) {
		// Assess whether or not this should be bounced by taking the intensity
		// of the diffuse component of the material
		float partition = current_material->get_diffuse_strength();
		float rgn		= (((float)(rand() % 10000)) /
					   10000.0f); // TODO: fix this, it is not portable
		if (rgn > partition) {
			// Bounce the photon off this material
			bounce_photon(current_material,
						  photon,
						  vp_tree,
						  max_depth,
						  world,
						  partition,
						  storage_mutex);
		}
		// TODO: Add contribution to nearby VP's if no bounce!!!
		std::vector<std::shared_ptr<poly::object::Object>> nearby_VPs =
			vp_tree.get_nearest_to_point(photon.point(),
										 max_distance_to_visible_point);
		for (auto &vp : nearby_VPs) {
			vp->add_contribution(photon, storage_mutex);
		}
		return;
	}
	else if (current_material->m_type ==
			 poly::structures::InteractionType::REFLECT) {
		float specular_kd	= current_material->get_specular_strength();
		float reflective_kd = current_material->get_reflective_strength();
		float diffuse_kd	= current_material->get_diffuse_strength();
		float total			= reflective_kd + diffuse_kd + specular_kd;

		float rgn = (((float)(rand() % 10000)) / 10000.0f) * total;

		if (rgn < reflective_kd) {
			bounce_photon(current_material,
						  photon,
						  vp_tree,
						  max_depth,
						  world,
						  (photon.intensity() * reflective_kd / total),
						  storage_mutex);
		}
		photon.intensity(photon.intensity() * (1 - (reflective_kd / total)));
		// photons.push_back(photon);
	}
	else if (current_material->m_type ==
			 poly::structures::InteractionType::TRANSMIT) {
		float transparent_kt = current_material->get_refractive_strength();
		float specular_kd	 = current_material->get_specular_strength();
		float reflective_kd	 = current_material->get_reflective_strength();
		float diffuse_kd	 = current_material->get_diffuse_strength();
		float total = transparent_kt + specular_kd + reflective_kd + diffuse_kd;

		// Random number in the range 0 to total
		float random_number = (((float)(rand() % 10000)) / 10000.0f) * total;

		if (random_number < transparent_kt) {
			transmit_photon(current_material,
							photon,
							vp_tree,
							max_depth,
							world,
							photon.intensity() * transparent_kt / total,
							storage_mutex);
		}
		else if (random_number >= transparent_kt &&
				 random_number < transparent_kt + reflective_kd) {
			bounce_photon(current_material,
						  photon,
						  vp_tree,
						  max_depth,
						  world,
						  (reflective_kd + reflective_kd) / total *
							  photon.intensity(),
						  storage_mutex);
		}
		photon.intensity(photon.intensity() * diffuse_kd / total);
		// Add photon contribution to VP
	}
}

void bounce_photon(
	[[maybe_unused]] std::shared_ptr<poly::material::Material> current_material,
	poly::structures::Photon &photon,
	poly::structures::KDTree &vp_tree,
	std::size_t max_depth,
	poly::structures::World const &world,
	float object_colour_intensity,
	std::shared_ptr<std::mutex> storage_mutex)
{
	poly::structures::SurfaceInteraction si;
	// Get the new ray direction from the photon
	atlas::math::Ray<atlas::math::Vector> photon_ray = photon.reflect_ray();

	// Hit new objects with this ray
	bool is_hit{false};
	for (auto &obj : world.m_scene) {
		if (obj->hit(photon_ray, si)) {
			is_hit = true;
		}
	}

	// If we hit an object, get its material and propogate this photon
	if (is_hit) {
		poly::structures::Photon reflected_photon = poly::structures::Photon(
			photon_ray,
			si.get_hitpoint(),
			si.m_normal,
			photon.intensity() * (1 - object_colour_intensity),
			photon.depth() + 1);
		absorb_photon(si.m_material,
					  reflected_photon,
					  vp_tree,
					  max_depth,
					  world,
					  storage_mutex);
	}
	float new_intensity = photon.intensity() * object_colour_intensity;
	photon.intensity(new_intensity);
}

void transmit_photon(std::shared_ptr<poly::material::Material> current_material,
					 poly::structures::Photon &photon,
					 poly::structures::KDTree &vp_tree,
					 std::size_t max_depth,
					 poly::structures::World const &world,
					 float colour_change,
					 std::shared_ptr<std::mutex> storage_mutex)
{
	poly::structures::SurfaceInteraction si;
	si.m_normal			   = photon.normal();
	atlas::math::Vector wi = -photon.wi().d;
	atlas::math::Vector wt;

	current_material->sample_f(si, wi, wt);

	atlas::math::Ray<atlas::math::Vector> photon_ray{photon.point(), wt};

	// Send the transmitted ray through the scene
	bool is_hit{false};
	for (auto &obj : world.m_scene) {
		if (obj->hit(photon_ray, si))
			is_hit = true;
	}

	// If we hit an object, possibly generate new rays, otherwise, simply change
	// the photons intensity
	if (is_hit) {
		poly::structures::Photon reflected_photon =
			poly::structures::Photon(photon_ray,
									 si.get_hitpoint(),
									 si.m_normal,
									 photon.intensity() * (1 - colour_change),
									 photon.depth() + 1);
		absorb_photon(si.m_material,
					  reflected_photon,
					  vp_tree,
					  max_depth,
					  world,
					  storage_mutex);
	}
	float new_intensity = photon.intensity() * colour_change;
	photon.intensity(new_intensity);
}
