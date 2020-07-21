#pragma once
#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <atlas/math/math.hpp>
#include "structures/surface_interaction.hpp"
#include "structures/world.hpp"

namespace poly::light
{
	class Light
	{
	public:
		Light();
		virtual atlas::math::Vector get_direction(poly::structures::SurfaceInteraction& sr) = 0;
		virtual Colour L(poly::structures::SurfaceInteraction& sr, poly::structures::World const& world) = 0;
		float ls() const;

		void radiance_scale(float b);

		void colour_set(Colour const& c);

		virtual atlas::math::Point location() const = 0;

	protected:
		float m_ls;
		Colour m_colour;
		float m_surface_epsilon;

		virtual bool in_shadow(atlas::math::Ray<atlas::math::Vector> const& shadow_ray,
			poly::structures::World const& world);
	};
} // namespace poly::light

#endif // !LIGHT_HPP