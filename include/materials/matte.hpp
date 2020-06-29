#pragma once

#include <memory>

#include "lights/light.hpp"
#include "textures/texture.hpp"
#include "BRDFs/lambertian.hpp"
#include "structures/KDTree.hpp"
#include "structures/world.hpp"

namespace poly::material
{

	class Matte : public Material
	{
	public:
		Matte();
		Matte(float f, Colour const &c);

		virtual float get_diffuse_strength() const;
		virtual float get_specular_strength() const;
		virtual float get_reflective_strength() const;
		virtual float get_refractive_strength() const;

		void absorb_photon(structures::Photon &photon, poly::structures::KDTree& vp_tree, unsigned int max_depth,
			poly::structures::World const& world) const;


		virtual void handle_vision_point(std::shared_ptr<poly::object::Object>& visible_point,
										 poly::structures::SurfaceInteraction& si,
										 poly::structures::World& world) const;

		poly::material::InteractionType sample_interation();

	protected:
		std::shared_ptr<LambertianBRDF> m_diffuse;

		Colour shade(poly::structures::SurfaceInteraction&sr, poly::structures::World const& world) const;


	};

} // namespace poly::material