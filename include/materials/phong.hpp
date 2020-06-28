#pragma once
#ifndef PHONGMAT_HPP
#define PHONGMAT_HPP

#include "BRDFs/glossy_specular.hpp"
#include "BRDFs/lambertian.hpp"
#include "structures/world.hpp"
#include "materials/material.hpp"
#include "structures/KDTree.hpp"
#include "lights/light.hpp"

namespace poly::material
{

	class Phong : public Material
	{
	public:
		Phong();
		Phong(float f_diffuse, float f_spec, Colour c, float exp);

		virtual Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const;
		void absorb_photon(structures::Photon &photon, poly::structures::KDTree& vp_tree, unsigned int max_depth,
			poly::structures::World const& world) const;
		virtual void handle_vision_point(std::shared_ptr<poly::object::Object>& visible_point,
										 poly::structures::SurfaceInteraction& si,
										 poly::structures::World& world) const;
	protected:
		std::shared_ptr<LambertianBRDF> m_diffuse;
		std::shared_ptr<GlossySpecularBRDF> m_specular;

	};
} // namespace poly::material
#endif // !PHONGMAT_HPP