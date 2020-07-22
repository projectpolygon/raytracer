#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

//#include "objects/object.hpp"
#include "structures/world.hpp"
#include "cameras/pinhole.hpp"
#include "materials/material.hpp"
#include "structures/scene_slab.hpp"
#include "cameras/pinhole.hpp"
#include "structures/bounds.hpp"
#include "structures/photon.hpp"
#include "lights/light.hpp"

namespace poly::integrators
{
	class VisiblePoint : public poly::object::Object
	{
	public:
		VisiblePoint(int x_,
					 int y_,
					 math::Point const& point_,
					 math::Vector const& incoming_ray_,
					 Colour amount_,
					 std::shared_ptr<poly::material::Material> material_,
					 std::shared_ptr<std::vector<std::vector<Colour>>> storage,
					 std::shared_ptr<poly::structures::World> world);
		// Provided to allow compatibility with Object type
		bool hit(math::Ray<math::Vector>const& R,
			poly::structures::SurfaceInteraction& sr) const;
		bool shadow_hit(math::Ray<math::Vector>const& R, float& t) const;
		void add_contribution(poly::structures::Photon const& photon,
							  std::shared_ptr<std::mutex> storage_mutex);

		// Originating Pixels
		int index_x; // Along the x axis
		int index_y; // Along the y axis

		// Point where the VisiblePoint was created
		atlas::math::Point point;

		// Direction of the incoming ray ( points toward the camera!)
		atlas::math::Vector w_i;

		// Amount of the photon's luminance we allow to contribute to this point
		Colour amount;

		// Material of the object that the VisiblePoint is on
		std::shared_ptr<poly::material::Material> surface_material;

		// Storage where the data should be stored
		std::shared_ptr<std::vector<std::vector<Colour>>> m_storage;

		// World used for render
		std::shared_ptr<poly::structures::World> m_world;
	};

	class SPPMIntegrator
	{
	public:
		SPPMIntegrator(std::size_t);
		void render(poly::structures::World const& world,
					poly::camera::PinholeCamera const& camera,
					poly::utils::BMP_info& output);

	private:
		std::size_t m_number_iterations;
		std::vector<std::shared_ptr<poly::object::Object>>
		create_visible_points(int start_x, int start_y, int end_x, int end_y,
							 std::shared_ptr<std::vector<std::vector<Colour>>> storage,
							 poly::camera::PinholeCamera const &camera,
							 std::shared_ptr<poly::structures::World> world);

		void photon_mapping(const structures::World &world, std::vector<std::shared_ptr<poly::object::Object>>& vp_list, std::shared_ptr<std::mutex> storage_mutex);
	};
} // namespace poly::integrators
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
#endif // !INTEGRATOR_HPP
