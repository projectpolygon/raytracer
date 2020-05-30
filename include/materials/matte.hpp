#pragma once

#include <memory>

#include <lights/light.hpp>
#include <textures/texture.hpp>
#include <BRDFs/lambertian.hpp>
#include <textures/constant_colour.hpp>
#include <textures/image_texture.hpp>

namespace poly::material
{

	class Matte : public Material
	{
	public:
		Matte();
		Matte(float f, Colour const &c);

	protected:
		std::shared_ptr<LambertianBRDF> m_diffuse;

		Colour shade(poly::structures::ShadeRec &sr) const;
	};

} // namespace poly::material