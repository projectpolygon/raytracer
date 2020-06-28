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

		void absorb_photon(structures::Photon &photon, poly::structures::KDTree& vp_tree, unsigned int max_depth,
			poly::structures::World const& world) const;

	protected:
		std::shared_ptr<LambertianBRDF> m_diffuse;

		Colour shade(poly::structures::SurfaceInteraction&sr, poly::structures::World const& world) const;


	};

} // namespace poly::material