#pragma once
#ifndef PHONGMAT_HPP
#define PHONGMAT_HPP

#include "BRDFs/glossy_specular.hpp"
#include "BRDFs/lambertian.hpp"
#include "structures/world.hpp"

namespace poly::material
{

	class Phong : public Material
	{
	public:
		Phong();
		Phong(float f_diffuse, float f_spec, Colour c, float exp);

	protected:
		std::shared_ptr<LambertianBRDF> m_diffuse;
		std::shared_ptr<GlossySpecularBRDF> m_specular;

		virtual Colour shade(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) const;
	};
} // namespace poly::material
#endif // !PHONGMAT_HPP