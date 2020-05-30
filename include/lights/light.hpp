#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "structures/shade_rec.hpp"
#include "structures/world.hpp"
#include "objects/object.hpp"

namespace poly::structures { class ShadeRec; }

namespace poly::light
{
	class Light
	{
	public:
		Light();
		virtual atlas::math::Vector direction_get(poly::structures::ShadeRec &sr) = 0;
		virtual Colour L(poly::structures::ShadeRec &sr) = 0;

		void radiance_scale(float b);

		void colour_set(Colour const &c);

	protected:
		float m_ls;
		Colour m_colour;
		float m_surface_epsilon;

		virtual bool in_shadow(math::Ray<math::Vector> const &shadow_ray,
							   poly::structures::ShadeRec const &sr);
	};
} // namespace poly::light

#endif // !LIGHT_HPP