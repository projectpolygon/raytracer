#pragma once
#ifndef SVMATTE_HPP
#define SVMATTE_HPP

#include <memory>

#include "structures/world.hpp"
#include "lights/light.hpp"
#include "textures/texture.hpp"
#include "BRDFs/lambertian.hpp"
#include "BRDFs/SV_lambertian.hpp"
#include "textures/constant_colour.hpp"
#include "textures/image_texture.hpp"

namespace poly::material
{

	class SV_Matte : public Material
	{
	public:
		SV_Matte(float f, std::shared_ptr<poly::texture::Texture> tex);

		SV_Matte(float f, Colour const &c);

		SV_Matte(float f, std::string const &s);

		void absorb_photon(structures::Photon &photon, std::vector<poly::structures::Photon> &photons, unsigned int max_depth,
						   std::vector<std::shared_ptr<poly::object::Object>> scene) const;

	protected:
		std::shared_ptr<SV_LambertianBRDF> m_diffuse;

		Colour shade(poly::structures::SurfaceInteraction&sr, poly::structures::World& world) const;
	};
} // namespace poly::material
#endif // !SVMATTE_HPP