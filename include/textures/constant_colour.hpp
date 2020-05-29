#pragma once

#include "textures/texture.hpp"
#include "structures/shade_rec.hpp"

namespace poly::texture {

    // Constant colour is the default texture (really, NO texture)
    class ConstantColour : public Texture {
    public:
        ConstantColour(Colour const& c);
        Colour colour_get([[maybe_unused]] poly::structures::ShadeRec const& sr) const;
        void colour_set(Colour const& c);
    private:
        Colour m_colour;
    };
}