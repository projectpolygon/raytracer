#include "materials/SV_matte.hpp"
#include "objects/object.hpp"

namespace poly::material {

	SV_Matte::SV_Matte(float f, std::shared_ptr<poly::texture::Texture> tex)
	{
		m_diffuse = std::make_shared<SV_LambertianBRDF>(f, tex);
	}

	SV_Matte::SV_Matte(float f, Colour const& c)
	{
		m_diffuse = std::make_shared<SV_LambertianBRDF>(f, std::make_shared<poly::texture::ConstantColour>(c));
	}

	SV_Matte::SV_Matte(float f, std::string const& s)
	{
		m_diffuse = std::make_shared<SV_LambertianBRDF>(f, std::make_shared<poly::texture::ImageTexture>(s));
	}

	Colour SV_Matte::shade(poly::structures::SurfaceInteraction& sr, poly::structures::World& world) const {
		// Render loop
		Colour r = Colour(0.0f, 0.0f, 0.0f);
		Colour a;
		atlas::math::Vector nullVec(0.0f, 0.0f, 0.0f);

		if (world.m_ambient) {
			a = m_diffuse->rho(sr, nullVec)
				* world.m_ambient->L(sr, world);
		}

		for (std::shared_ptr<poly::light::Light> light : world.m_lights) {
			Colour brdf = m_diffuse->f(sr, nullVec, nullVec);
			Colour L = light->L(sr, world);
			float angle = glm::dot(sr.m_normal,
				light->direction_get(sr));
			if (angle >= 0) {
				r += (brdf
					* L
					* angle);
			}
			else {
				r += Colour(0.0f, 0.0f, 0.0f);
			}
		}

		return (a + r);
	}

	void SV_Matte::absorb_photon([[maybe_unused]] structures::Photon &p, [[maybe_unused]] poly::structures::KDTree& vp_tree,
								 [[maybe_unused]] unsigned int max_depth, [[maybe_unused]] std::vector<std::shared_ptr<poly::object::Object>> scene) const {

	}

	void SV_Matte::handle_vision_point(std::shared_ptr<poly::object::Object> &visible_point,
									   structures::SurfaceInteraction &si, structures::World &world) const
	{
		(void) visible_point;
		(void) si;
		(void) world;
	}
}
