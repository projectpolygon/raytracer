#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "structures/surface_interaction.hpp"
#include "structures/world.hpp"
#include "objects/object.hpp"

namespace poly::light
{
	class Light
	{
	public:
		Light();
		virtual atlas::math::Vector get_direction(poly::structures::SurfaceInteraction& sr) = 0;
		virtual Colour L(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) = 0;

		void radiance_scale(float b);

		void colour_set(Colour const& c);

	protected:
		float m_ls;
		Colour m_colour;
		float m_surface_epsilon;

		virtual bool in_shadow(math::Ray<math::Vector> const& shadow_ray,
			poly::structures::World const& world);
	};
} // namespace poly::light

#endif // !LIGHT_HPP