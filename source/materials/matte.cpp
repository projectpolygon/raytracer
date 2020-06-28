#include "materials/matte.hpp"
#include "structures/KDTree.hpp"
#include "textures/image_texture.hpp"
#include "textures/constant_colour.hpp"

namespace poly::material
{

	Matte::Matte()
	{
		m_diffuse = std::make_shared<LambertianBRDF>(
			1.0f,
			poly::utils::random_colour_generate());
	}
	Matte::Matte(float f, Colour const &c)
	{
		m_diffuse = std::make_shared<LambertianBRDF>(f, c);
	}

	Colour Matte::shade(poly::structures::SurfaceInteraction &sr, poly::structures::World const& world) const
	{
		// Render loop
		Colour r = Colour(0.0f, 0.0f, 0.0f);
		Colour a = Colour(0.0f, 0.0f, 0.0f);
		atlas::math::Vector nullVec(0.0f, 0.0f, 0.0f);

		if (world.m_ambient)
		{
			a = m_diffuse->rho(sr, nullVec) * world.m_ambient->L(sr, world);
		}

		for (std::shared_ptr<poly::light::Light> light : world.m_lights)
		{
			Colour brdf = m_diffuse->f(sr, nullVec, nullVec);
			Colour L = light->L(sr, world);
			float angle = glm::dot(sr.m_normal, 
				light->direction_get(sr));
			if (angle > 0)
			{
				r += (brdf * L * angle);
			}
			else
			{
				r += Colour(0.0f, 0.0f, 0.0f);
			}
		}

		return (a + r);
	}

	void Matte::absorb_photon(structures::Photon &photon, poly::structures::KDTree& vp_tree,
							  unsigned int max_depth, poly::structures::World const& world) const {
		if (photon.depth() >= max_depth) {
			//photons.push_back(photon);
			//std::vector<std::shared_ptr<poly::object::VisiblePoint>> nearby_VPs = vp_tree.get_nearest_to_point(photon.point, 2.0f, 5);
			//for (auto vp : nearby_VPs) {
				//vp->add_contribution(photon);
			//}
			// Add contribution to nearby VP's
			return;
		}

		float partition = m_diffuse->kd();
		float rgn = (float(rand()) / float(std::numeric_limits<int>::max()));
		if (rgn > partition) {
			bounce_photon(photon, vp_tree, max_depth, world, partition);
		}
		// Add contribution to nearby VP's
		//photons.push_back(photon);
	}

	void Matte::handle_vision_point(std::shared_ptr<poly::object::Object> &visible_point, structures::SurfaceInteraction &si,
							   structures::World &world) const
	{
		(void) visible_point;
		(void) si;
		(void) world;
	}

	poly::material::InteractionType Matte::sample_interation()
	{
		float partition = m_diffuse->kd();
		float rgn = (float(rand()) / float(std::numeric_limits<int>::max()));
		if (rgn > partition) {
			return poly::material::InteractionType::REFLECT;
		}
		return poly::material::InteractionType::ABSORB;
	}
} // namespace poly::material
