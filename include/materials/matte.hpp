#pragma once

#include <memory>

#include "lights/light.hpp"
#include "textures/texture.hpp"
#include "BRDFs/lambertian.hpp"
#include "textures/constant_colour.hpp"
#include "textures/image_texture.hpp"
#include "structures/world.hpp"

namespace poly::material
{

	class Matte : public Material
	{
	public:
		Matte();
		Matte(float f, Colour const &c);

		void absorb_photon(structures::Photon &photon, std::vector<poly::structures::Photon> &photons, unsigned int max_depth,
						   std::vector<std::shared_ptr<poly::object::Object>> scene) const;

	protected:
		std::shared_ptr<LambertianBRDF> m_diffuse;

		Colour shade(poly::structures::SurfaceInteraction&sr, poly::structures::World const& world) const;


	};

} // namespace poly::material