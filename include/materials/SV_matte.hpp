#pragma once
#ifndef SVMATTE_HPP
#define SVMATTE_HPP

#include <memory>

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

	protected:
		std::shared_ptr<SV_LambertianBRDF> m_diffuse;

		Colour shade(poly::structures::ShadeRec &sr) const;
	};
} // namespace poly::material
#endif // !SVMATTE_HPP