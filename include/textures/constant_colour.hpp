#pragma once

#include "textures/texture.hpp"
#include "structures/surface_interaction.hpp"

namespace poly::texture {

	// Constant colour is the default texture (really, NO texture)
	class ConstantColour : public Texture {
	public:
		ConstantColour(Colour const& c);
		Colour colour_get([[maybe_unused]] poly::structures::SurfaceInteraction const& sr) const;
		void colour_set(Colour const& c);
	private:
		Colour m_colour;
	};
}