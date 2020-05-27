#pragma once

namespace poly::texture {

    // Constant colour is the default texture (really, NO texture)
    class ConstantColour : public Texture {
    public:
        ConstantColour() = default;
        ConstantColour(Colour const& c) :m_colour{c} {}
        Colour colour_get([[maybe_unused]] poly::structures::ShadeRec const& sr) const {
          return m_colour;
        }
        void colour_set(Colour const& c) {
          m_colour = c;
        }
    private:
        Colour m_colour;
    };
}