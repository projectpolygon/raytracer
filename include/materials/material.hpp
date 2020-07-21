#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <atlas/math/math.hpp>
#include "structures/photon.hpp"
#include "structures/world.hpp"
#include "structures/KDTree.hpp"

namespace poly::structures { class KDTree; }
namespace poly::structures { class SurfaceInteraction; enum class InteractionType; }
namespace poly::structures { class Photon; }

namespace poly::material {

	/*enum InteractionType {
		ABSORB,
		REFLECT,
		TRANSMIT,
		NUM_INTERACTION_TYPES
	}; // TODO possibly move this somewhere else?*/

	class Material {
	public:
		Material();
		virtual atlas::math::Vector shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const = 0;
		
		enum poly::structures::InteractionType m_type;

		virtual float get_diffuse_strength() const = 0;
		virtual float get_specular_strength() const = 0;
		virtual float get_reflective_strength() const = 0;
		virtual float get_refractive_strength() const = 0;
		virtual atlas::math::Vector get_hue([[maybe_unused]] atlas::math::Point const& hp) const = 0;

		virtual atlas::math::Vector sample_f(poly::structures::SurfaceInteraction const& sr, atlas::math::Vector& w_o, atlas::math::Vector& w_t) const;
		
		/*
		virtual void absorb_photon(poly::structures::Photon& photon, poly::structures::KDTree& vp_tree, unsigned int max_depth,
			poly::structures::World const& world) const = 0;
		virtual void bounce_photon(poly::structures::Photon& photon, poly::structures::KDTree& vp_tree, unsigned int max_depth,
			poly::structures::World const& world, float object_colour_intensity) const;
		virtual void handle_vision_point(std::shared_ptr<poly::object::Object>& visible_point,
			poly::structures::SurfaceInteraction& si,
			poly::structures::World& world) const = 0;
		*/
	};
}

#endif // !MATERIAL_HPP
