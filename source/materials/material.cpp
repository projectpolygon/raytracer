#include "materials/material.hpp"

namespace poly::material
{
	Material::Material() : m_type{poly::structures::InteractionType::ABSORB}
	{}

	Colour Material::sample_f(
		[[maybe_unused]] poly::structures::SurfaceInteraction const& sr,
		[[maybe_unused]] atlas::math::Vector& w_o,
		[[maybe_unused]] atlas::math::Vector& w_t) const
	{
		return Colour{};
	}

	Colour Material::get_hue([
		[maybe_unused]] atlas::math::Point const& hp) const
	{
		return Colour{0.0f, 0.0f, 0.0f};
	}
} // namespace poly::material
