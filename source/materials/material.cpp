#include "materials/material.hpp"

namespace poly::material {

	void poly::material::Material::bounce_photon(structures::Photon &photon, std::vector <poly::structures::Photon> &photons,
		unsigned int max_depth, std::vector <std::shared_ptr<poly::object::Object>> scene, float object_colour_intensity) const {
		poly::structures::SurfaceInteraction si;
		atlas::math::Ray<atlas::math::Vector> photon_ray = photon.reflect_ray();
		bool is_hit{false};

		for (auto obj: scene) {
			if (obj->hit(photon_ray, si))
				is_hit = true;
		}

		if (is_hit) {
			poly::structures::Photon reflected_photon = poly::structures::Photon(photon_ray,
				si.hitpoint_get(), si.m_normal, photon.intensity() * (1 - object_colour_intensity), photon.depth() + 1);
			si.m_material->absorb_photon(reflected_photon, photons, max_depth, scene);
		}
		float new_intensity = photon.intensity() * object_colour_intensity;
		photon.intensity(new_intensity);
	}
}
