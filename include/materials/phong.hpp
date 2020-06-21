#pragma once
#ifndef PHONGMAT_HPP
#define PHONGMAT_HPP

#include "BRDFs/glossy_specular.hpp"
#include "BRDFs/lambertian.hpp"
#include "structures/world.hpp"
#include "materials/material.hpp"
#include "lights/light.hpp"

namespace poly::material
{

	class Phong : public Material
	{
	public:
		Phong();
		Phong(float f_diffuse, float f_spec, Colour c, float exp);

		virtual Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const;
		void trace_photon(const poly::structures::Photon& p, std::vector<poly::structures::Photon>& photons,
						  unsigned int max_depth) const;
	protected:
		std::shared_ptr<LambertianBRDF> m_diffuse;
		std::shared_ptr<GlossySpecularBRDF> m_specular;

	};
} // namespace poly::material
#endif // !PHONGMAT_HPP