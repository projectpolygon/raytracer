#pragma once

#include <atlas/math/math.hpp>
#include "lights/light.hpp"

namespace poly::light
{

	class AmbientOcclusion : public Light
	{
	public:
		AmbientOcclusion();

		void min_amount_set(float amount);

		void sampler_set(std::shared_ptr<poly::sampler::Sampler> s_ptr, float tightness);

		math::Vector get_direction([[maybe_unused]] poly::structures::SurfaceInteraction&sr);

		// Specific to ambient occlusion, we need access to samples mapped to hemisphere
		math::Vector get_shadow_direction([[maybe_unused]] poly::structures::SurfaceInteraction&sr, unsigned int sample_index);

		Colour L(poly::structures::SurfaceInteraction &sr, poly::structures::World const& world);

	protected:
		std::shared_ptr<poly::sampler::Sampler> m_sampler;
		Colour min_amount;
	};
} // namespace poly::light