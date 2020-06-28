#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "structures/photon.hpp"
#include "structures/world.hpp"
#include "structures/KDTree.hpp"

namespace poly::structures { class KDTree; }
namespace poly::structures { class SurfaceInteraction; }
namespace poly::structures { class Photon; }

namespace poly::material {

	enum InteractionType {
		REFLECT,
		TRANSMIT,
		ABSORB
	};

	class Material {
	public:
		Material() = default;
		virtual atlas::math::Vector shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const = 0;
		virtual void absorb_photon(poly::structures::Photon& photon, poly::structures::KDTree& vp_tree, unsigned int max_depth,
			poly::structures::World const& world) const = 0;
		virtual void bounce_photon(poly::structures::Photon& photon, poly::structures::KDTree& vp_tree, unsigned int max_depth,
			poly::structures::World const& world, float object_colour_intensity) const;
		virtual void handle_vision_point(std::shared_ptr<poly::object::Object>& visible_point,
			poly::structures::SurfaceInteraction& si,
			poly::structures::World& world) const = 0;
	};
}

#endif // !MATERIAL_HPP