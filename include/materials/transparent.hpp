#pragma once

#include <memory>
#include "BRDFs/perfect_specular.hpp"
#include "BTDFs/perfect_transmitter.hpp"
#include "materials/phong.hpp"
#include "tracers/tracer.hpp"
#include "structures/KDTree.hpp"

namespace poly::material {

	class Transparent : public Phong {
	public:
		Transparent();
		Transparent(const float amount_refl,
			const float amount_trans,
			float f_diffuse,
			float f_spec,
			Colour const& _colour,
			float _ior,
			float _exp);

		Colour sample_f(poly::structures::SurfaceInteraction const& sr,
			atlas::math::Vector& w_o,
			atlas::math::Vector& w_t) const;

		Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const;

		virtual float get_diffuse_strength() const;
		virtual float get_specular_strength() const;
		virtual float get_reflective_strength() const;
		virtual float get_refractive_strength() const;
		virtual Colour get_hue([[maybe_unused]] atlas::math::Point& hp) const;

		void absorb_photon(poly::structures::Photon& p, poly::structures::KDTree& vp_tree,
			unsigned int max_depth, poly::structures::World& world) const;

		void transmit_photon(poly::structures::Photon& p, poly::structures::KDTree& vp_tree,
			unsigned int max_depth, poly::structures::World& world,
			float intensity) const;
		virtual void handle_vision_point(std::shared_ptr<poly::object::Object>& visible_point,
										 poly::structures::SurfaceInteraction& si,
										 poly::structures::World& world) const;

	protected:
		std::shared_ptr<PerfectSpecular> m_reflected_brdf;
		std::shared_ptr<PerfectTransmitter> m_transmitted_btdf;
	};

}