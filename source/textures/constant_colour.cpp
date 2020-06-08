#include "textures/constant_colour.hpp"

namespace poly::texture
{

	ConstantColour::ConstantColour(Colour const &c) : m_colour{c} {}
	Colour ConstantColour::colour_get([[maybe_unused]] poly::structures::ShadeRec const &sr) const
	{
		return m_colour;
	}
	void ConstantColour::colour_set(Colour const &c)
	{
		m_colour = c;
	}
} // namespace poly::texture
